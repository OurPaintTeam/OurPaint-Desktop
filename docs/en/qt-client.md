# Qt client integration

The product works with documents, not projects. Each `.ourp` stores one document
using the existing Core CBOR codec. `Application` composes `QApplication`, one
`app::UIController`, and `QtUIController`. The Qt client does not access
`Document`, `DocumentManager`, `DocumentView`, or `SketchEditor` directly.

## Ownership and entry points

- `UIController` owns documents and independent views. `createDocumentView()`
  always creates a new view; `openDocument()` reuses an already-open path.
- `findViewport(ViewId)` returns a borrowed `ViewportController`. Its lifetime
  ends at `closeDocumentView()` or `closeDocument()`.
- `QtUIController` owns UI window managers and maps positive UI tab tokens to
  `ViewId`. Negative recent-list tokens identify paths, not open documents/views.
- Each native window has a `QtViewport` (`QOpenGLWindow`) and its own context.
  The window container owns the surface after embedding. Inactive tabs may retain
  GPU resources in that context; the adapter tracks them for cleanup.
- Switching tabs chooses the visible controller. Moving a tab retains its ViewId,
  camera and editor, but shuts down its renderer in the source context before
  using the destination context. A document can be visible in several windows.
- `UIObserver` adapts optional synchronous App notifications to Qt signals. The
  close notification must be direct: it releases the viewport while App still
  owns it. Observers may query App but cannot re-enter document/view mutations.
- Input uses logical pixel coordinates; resize also supplies the current DPR.
  Qt makes the context current for painting. Rendering is scheduled with
  `update()`; continuous rendering schedules another frame after `frameSwapped`.

## Document lifecycle and persistence

Startup creates an in-memory `Untitled` with one view. Creation and closing never
write document contents implicitly. An unchanged empty draft can be discarded
without a prompt. Before closing the last view of an edited document, Qt asks
Save / Discard / Cancel. Closing one of several views preserves the document.
Closing a window asks all necessary questions before destroying any views.

Save writes only the active document. Save As selects its new path. Both use
`NativeFile`'s temporary-file replacement; a failed save does not change the
in-memory destination or saved-state snapshot. If final directory synchronization
fails after replacement, an error is still reported conservatively. The existing
Core codec is reused, so this integration does not introduce another CAD format.

`Document::isDirty()` currently tracks mutable access, including some reads by the
editor, rather than a reliable content revision. `documentModified()` compares
serialized contents with the last saved/loaded snapshot (or initial draft). This
is only used at explicit close decisions, never per frame. A Core revision model
would eventually avoid the extra snapshot memory and serialization cost.

One `.ourpaint` is stored under `QStandardPaths::AppConfigLocation`. The frontend
supplies its absolute path to App. Version 1 contains `version` and
`recentDocuments`: up to 20 absolute UTF-8 paths, newest first. It contains no
unsaved document contents and does not restore a session automatically. Missing
metadata is normal; corrupt/future metadata is preserved and a warning is shown.
Metadata write errors do not turn a successful document save into a failure.

The legacy `Project`/`ProjectSaveLoad` helpers and their tests remain in the source
tree, but the Qt client and UIController no longer use their directory-project
model. `FileResult.h` contains the shared file error/result types. CMake composes
UIController and DocumentView in `ourpaint_project` to avoid a dependency cycle;
target/folder renaming is a separate cleanup.

## UI library changes and remaining API issues

The integration requires changes in the `third_party/UI` submodule:

- `ProjectManager::setRecentDocuments()` supplies an external catalog. Null
  `EditorContext::fileSystem` now works; no FileSystem or SQLite catalog is created
  by the Qt client. Default dialog directories use Documents.
- ProjectManager owns its windows and their embedded surfaces. Detached-window
  return is a request; it no longer destroys itself before the client can transfer
  the view. Windows do not delete themselves on Close. QtUIController intercepts
  Close to allow cancellation, then disposes of the manager after event delivery.
  The UI demo client also disposes of a closed detached manager explicitly.
- The detached settings callback guards the absent main window. MainWindow no
  longer emits its close notification twice for one close action.
- `setActiveTab()` now changes the actual selected tab and reveals the editor.
  Previously it only changed the command recipient ID. Empty tab bars return -1.
  Re-selecting an already active tab does not emit another activation; renaming
  a tab does not change the command recipient.
- New-document dialogs request a name, defaulting to Untitled, instead of a folder.
  Console forwarding now accepts a valid active ID (the old check was inverted).

These issues still need UI/API work:

1. Names such as ProjectManager, ProjectData, projectMenuType and
   defaultProjectPath still imply projects. The recent catalog should use paths
   or explicit catalog IDs, while tab operations should use view tokens.
2. `removeFileTriggered` conflates closing a view, forgetting a recent path and
   deleting a file. The client reports the operation as unsupported; it never
   deletes a user's file. Save As writes under a new filename and preserves the
   original file. Only drafts can be renamed directly. The UI should expose
   distinct requests.
3. Save and Save As need visible menu actions. Currently Save uses Ctrl+S and
   Save As uses Ctrl+Shift+S. Ctrl+N creates a draft, Ctrl+W closes a view, and
   Ctrl+Shift+N opens another view of the active document in a new window.
   Export still has no implemented App operation.
4. Close-all tab actions emit independent close requests and cannot stop the
   sequence after a cancellation. A cancellable batch request is needed for
   that UX. Individual cancelled documents remain open and unchanged.
5. UI SettingsSystem still writes `settings.ini` for its own preferences.
   Consolidating those preferences into `.ourpaint` needs a UI settings contract;
   this integration stores the App recent-document metadata in `.ourpaint`.
   Grid/snap controls currently have no corresponding App settings API.
6. Console/script execution and the Size tool have no implemented App commands.
   The adapter reports this rather than pretending to perform an operation.
   The UI CubicBezier item currently maps to App's Nurbs tool; terminology and
   available modes should be aligned with the actual editor capabilities.
7. The client enforces close cancellation through a native event filter because
   `windowClosed` is only a notification. An explicit close-request protocol would
   make the library contract clearer.

The renderer currently opens shaders/fonts relative to the working directory.
After resolving command-line paths, Application selects the executable directory,
where CMake copies those resources. An explicit render resource root can replace
this process-wide convention later. File association installation is left to
packaging; normal startup does not modify the Windows registry.

## Validation and manual follow-up

The affected Qt target and the UIControllerViewsGTEST executable (including the
new document persistence cases) are compiled. Tests and the application are not
run automatically. The document cases cover drafts, view/document lifetimes,
save/open identity, geometry preservation, failed saves, invalid input, duplicate destinations, recent
metadata and metadata errors.

Manual checks still required after an explicitly authorized launch:

- Create geometry, save, reopen and compare; test Unicode paths and Save As.
- Cancel a save dialog and each close prompt; confirm the document stays open.
- Close the last unchanged Untitled and verify no `.ourp` was created.
- Reopen an existing document from recents and verify both the visible tab and
  command recipient switch together.
- Open a second view with Ctrl+Shift+N; edit one and inspect the other.
- Detach, return and drag tabs between windows; verify cameras survive, closing
  windows does not lose another view, and GL resource cleanup does not crash.
- Move a window between monitors with different scaling; inspect drawing and input.
- Launch a second instance with a relative `.ourp` path and verify it reaches the
  first instance as an absolute document path.

Commit the UI submodule changes separately and update the parent repository's
submodule pointer before sharing the integration. A parent commit alone does not
include modified files inside the submodule.
