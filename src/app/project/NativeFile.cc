#include "NativeFile.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <string>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace app::detail {
namespace {

namespace fs = std::filesystem;

#ifdef _WIN32
using NativeHandle = HANDLE;
const NativeHandle InvalidHandle = INVALID_HANDLE_VALUE;

std::error_code lastError() { return {static_cast<int>(GetLastError()), std::system_category()}; }
#else
using NativeHandle = int;
constexpr NativeHandle InvalidHandle = -1;

std::error_code lastError() { return {errno, std::generic_category()}; }
#endif

class FileHandle {
public:
    explicit FileHandle(NativeHandle handle = InvalidHandle) : handle_(handle) {}
    ~FileHandle() { close(); }
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    NativeHandle get() const { return handle_; }

    std::error_code close() {
        if (handle_ == InvalidHandle) {
            return {};
        }
#ifdef _WIN32
        if (!CloseHandle(handle_)) {
            return lastError();
        }
        handle_ = InvalidHandle;
#else
        // On Linux even a failed close releases the descriptor. Retrying could
        // close a descriptor subsequently allocated by another thread.
        const auto handle = std::exchange(handle_, InvalidHandle);
        if (::close(handle) != 0) {
            return lastError();
        }
#endif
        return {};
    }

private:
    NativeHandle handle_;
};

class TemporaryFile {
public:
    TemporaryFile(fs::path path, NativeHandle handle) : path_(std::move(path)), file_(handle) {}
    ~TemporaryFile() {
        file_.close();
        if (!published_) {
#ifdef _WIN32
            DeleteFileW(path_.c_str());
#else
            ::unlink(path_.c_str());
#endif
        }
    }
    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    const fs::path& path() const { return path_; }
    NativeHandle handle() const { return file_.get(); }
    std::error_code close() { return file_.close(); }
    void published() { published_ = true; }

private:
    fs::path path_;
    FileHandle file_;
    bool published_ = false;
};

FileReplacementResult failure(const fs::path& target, FileStage stage, std::error_code error, bool replaced = false) {
    return {FileError{target, stage, error, std::nullopt}, replaced};
}

fs::path parentDirectory(const fs::path& target) { return target.has_parent_path() ? target.parent_path() : fs::path("."); }

fs::path temporaryPath(const fs::path& parent) {
    static std::atomic<uint64_t> sequence{0};
#ifdef _WIN32
    const auto process = GetCurrentProcessId();
#else
    const auto process = ::getpid();
#endif
    return parent / (".ourpaint-tmp-" + std::to_string(process) + "-" + std::to_string(sequence.fetch_add(1, std::memory_order_relaxed)) + ".tmp");
}

NativeHandle createExclusive(const fs::path& path) {
#ifdef _WIN32
    return CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
    NativeHandle handle;
    do {
        handle = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0666);
    } while (handle == InvalidHandle && errno == EINTR);
    return handle;
#endif
}

bool alreadyExists(std::error_code error) {
#ifdef _WIN32
    return error.value() == ERROR_FILE_EXISTS || error.value() == ERROR_ALREADY_EXISTS;
#else
    return error == std::errc::file_exists;
#endif
}

std::error_code writeAll(NativeHandle handle, std::span<const uint8_t> bytes) {
    while (!bytes.empty()) {
        // Keep each request representable by both DWORD and ssize_t.
        const auto chunk = (std::min)(bytes.size(), size_t{1024 * 1024});
#ifdef _WIN32
        DWORD written = 0;
        if (!WriteFile(handle, bytes.data(), static_cast<DWORD>(chunk), &written, nullptr)) {
            return lastError();
        }
#else
        const auto written = ::write(handle, bytes.data(), chunk);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return lastError();
        }
#endif
        if (written == 0) {
            return std::make_error_code(std::errc::io_error);
        }
        bytes = bytes.subspan(static_cast<size_t>(written));
    }
    return {};
}

std::error_code flush(NativeHandle handle) {
#ifdef _WIN32
    if (!FlushFileBuffers(handle)) {
        return lastError();
    }
#else
    int result;
    do {
        result = ::fsync(handle);
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        return lastError();
    }
#endif
    return {};
}

}  // namespace

bool FilenameLess::operator()(const fs::path& first, const fs::path& second) const {
#ifdef _WIN32
    return CompareStringOrdinal(first.c_str(), -1, second.c_str(), -1, TRUE) == CSTR_LESS_THAN;
#else
    return first.native() < second.native();
#endif
}

std::optional<FileError> validateFileDestination(const fs::path& target) {
    const auto invalid = [&target](std::error_code error) -> std::optional<FileError> {
        return FileError{target, FileStage::ValidatePath, error, std::nullopt};
    };
    if (target.empty() || target.filename().empty() || target.filename() == "." || target.filename() == ".." ||
        target.native().find(fs::path::value_type{}) != fs::path::string_type::npos) {
        return invalid(std::make_error_code(std::errc::invalid_argument));
    }
    std::error_code error;
    const auto parent = fs::status(parentDirectory(target), error);
    if (error) {
        return invalid(error);
    }
    if (!fs::is_directory(parent)) {
        return invalid(std::make_error_code(std::errc::not_a_directory));
    }
    const auto status = fs::symlink_status(target, error);
    if (error && error != std::errc::no_such_file_or_directory) {
        return invalid(error);
    }
    if (status.type() != fs::file_type::not_found && !fs::is_regular_file(status)) {
        return invalid(std::make_error_code(std::errc::invalid_argument));
    }
    return std::nullopt;
}

FileReplacementResult replaceFile(const fs::path& target, std::span<const uint8_t> bytes) {
    if (auto error = validateFileDestination(target)) {
        return {std::move(error), false};
    }
    const auto parent = parentDirectory(target);
#ifndef _WIN32
    NativeHandle descriptor;
    do {
        descriptor = ::open(parent.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    } while (descriptor == InvalidHandle && errno == EINTR);
    if (descriptor == InvalidHandle) {
        return failure(target, FileStage::SyncDirectory, lastError());
    }
    FileHandle directory(descriptor);
#endif
    fs::path temporary;
    NativeHandle handle = InvalidHandle;
    for (unsigned attempt = 0; attempt < 128; ++attempt) {
        temporary = temporaryPath(parent);
        if (temporary == target) {
            continue;
        }
        handle = createExclusive(temporary);
        if (handle != InvalidHandle) {
            break;
        }
        const auto error = lastError();
        if (!alreadyExists(error)) {
            return failure(target, FileStage::CreateTemporary, error);
        }
    }
    if (handle == InvalidHandle) {
        return failure(target, FileStage::CreateTemporary, std::make_error_code(std::errc::file_exists));
    }
    TemporaryFile output(std::move(temporary), handle);
    if (const auto error = writeAll(output.handle(), bytes)) {
        return failure(target, FileStage::Write, error);
    }
    if (const auto error = flush(output.handle())) {
        return failure(target, FileStage::Flush, error);
    }
    if (const auto error = output.close()) {
        return failure(target, FileStage::Close, error);
    }
#ifdef _WIN32
    // No MOVEFILE_COPY_ALLOWED: replacement must remain on the same volume.
    if (!MoveFileExW(output.path().c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
#else
    if (::rename(output.path().c_str(), target.c_str()) != 0) {
#endif
        return failure(target, FileStage::Replace, lastError());
    }
    output.published();
#ifndef _WIN32
    if (const auto error = flush(directory.get())) {
        return failure(target, FileStage::SyncDirectory, error, true);
    }
    if (const auto error = directory.close()) {
        return failure(target, FileStage::SyncDirectory, error, true);
    }
#endif
    return {std::nullopt, true};
}

FileResult<core::persistence::DocumentBytes> readFile(const fs::path& path) {
    using ReadResult = FileResult<core::persistence::DocumentBytes>;
    const auto fail = [&path](FileStage stage, std::error_code error) {
        return ReadResult::failure({path, stage, error, std::nullopt});
    };
    const auto tooLarge = [&path]() {
        core::persistence::CodecError error{core::persistence::CodecErrorCode::LimitExceeded, "$", "Input exceeds the 64 MiB limit"};
        return ReadResult::failure({path, FileStage::Read, {}, std::move(error)});
    };
    if (auto error = validateFileDestination(path)) {
        return ReadResult::failure(std::move(*error));
    }
#ifdef _WIN32
    const auto handle = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
#else
    NativeHandle handle;
    do {
        handle = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
    } while (handle == InvalidHandle && errno == EINTR);
#endif
    if (handle == InvalidHandle) {
        return fail(FileStage::Open, lastError());
    }
    FileHandle input(handle);
    uint64_t size = 0;
#ifdef _WIN32
    BY_HANDLE_FILE_INFORMATION info{};
    if (!GetFileInformationByHandle(handle, &info)) {
        return fail(FileStage::Read, lastError());
    }
    if (info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) {
        return fail(FileStage::ValidatePath, std::make_error_code(std::errc::invalid_argument));
    }
    size = (uint64_t{info.nFileSizeHigh} << 32) | info.nFileSizeLow;
#else
    struct stat info {};
    if (::fstat(handle, &info) != 0) {
        return fail(FileStage::Read, lastError());
    }
    if (!S_ISREG(info.st_mode) || info.st_size < 0) {
        return fail(FileStage::ValidatePath, std::make_error_code(std::errc::invalid_argument));
    }
    size = static_cast<uint64_t>(info.st_size);
#endif
    if (size > core::persistence::MaxDocumentBytes) {
        return tooLarge();
    }
    core::persistence::DocumentBytes bytes;
    bytes.reserve(static_cast<size_t>(size));
    std::array<uint8_t, 64 * 1024> buffer;
    for (;;) {
#ifdef _WIN32
        DWORD count = 0;
        if (!ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &count, nullptr)) {
            return fail(FileStage::Read, lastError());
        }
#else
        const auto count = ::read(handle, buffer.data(), buffer.size());
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            return fail(FileStage::Read, lastError());
        }
#endif
        if (count == 0) {
            break;
        }
        if (static_cast<size_t>(count) > core::persistence::MaxDocumentBytes - bytes.size()) {
            return tooLarge();
        }
        bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(count));
    }
    if (const auto error = input.close()) {
        return fail(FileStage::Close, error);
    }
    return ReadResult::success(std::move(bytes));
}

}  // namespace app::detail
