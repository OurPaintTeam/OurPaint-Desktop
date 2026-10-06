#include "ConstraintActions.h"

#include <array>
#include <cmath>
#include <numbers>
#include <utility>

#include "../../core/sketch/Sketch.h"

namespace {
enum class RefKind { Invalid, Point, Line, Circular };

RefKind classifyRef(const core::sketch::SketchEntity& entity, core::sketch::SubElement sub) {
    using core::sketch::EntityKind;
    using core::sketch::SubElement;

    switch (core::sketch::entityKind(entity.geometry)) {
        case EntityKind::Point:
            return sub == SubElement::Whole ? RefKind::Point : RefKind::Invalid;
        case EntityKind::Line:
            if (sub == SubElement::Whole) {
                return RefKind::Line;
            }
            return sub == SubElement::Start || sub == SubElement::End ? RefKind::Point : RefKind::Invalid;
        case EntityKind::Circle:
            if (sub == SubElement::Whole) {
                return RefKind::Circular;
            }
            return sub == SubElement::Center ? RefKind::Point : RefKind::Invalid;
        case EntityKind::Arc:
            if (sub == SubElement::Whole) {
                return RefKind::Circular;
            }
            return sub == SubElement::Start || sub == SubElement::End || sub == SubElement::Center ? RefKind::Point : RefKind::Invalid;
    }
    return RefKind::Invalid;
}
}  // namespace

ConstraintActions::ConstraintActions(core::sketch::Sketch& sketch) : sketch_(sketch) {}

ConstraintPreparation ConstraintActions::prepare(const ConstraintRequest& request, std::span<const core::sketch::GeometryRef> refs) const {
    using core::sketch::ConstraintType;
    using State = ConstraintPreparation::State;

    const bool dimensional = request.action == ConstraintAction::Dimension || request.action == ConstraintAction::Angle;
    if (request.value.has_value() != dimensional) {
        return {State::InvalidSelection, std::nullopt};
    }
    if (dimensional && (!std::isfinite(*request.value) || *request.value < 0)) {
        return {State::InvalidSelection, std::nullopt};
    }
    if (request.action == ConstraintAction::Angle && *request.value > std::numbers::pi) {
        return {State::InvalidSelection, std::nullopt};
    }
    if (refs.size() > 2 || (refs.size() == 2 && refs[0] == refs[1])) {
        return {State::InvalidSelection, std::nullopt};
    }

    core::sketch::ConstraintDefinition definition;
    definition.value = request.value;
    size_t requiredRefs = 2;
    switch (request.action) {
        case ConstraintAction::Coincident:
            definition.type = ConstraintType::Coincident;
            break;
        case ConstraintAction::Horizontal:
            definition.type = ConstraintType::Horizontal;
            requiredRefs = 1;
            break;
        case ConstraintAction::Vertical:
            definition.type = ConstraintType::Vertical;
            requiredRefs = 1;
            break;
        case ConstraintAction::Parallel:
            definition.type = ConstraintType::Parallel;
            break;
        case ConstraintAction::Perpendicular:
            definition.type = ConstraintType::Perpendicular;
            break;
        case ConstraintAction::Tangent:
            definition.type = ConstraintType::Tangent;
            break;
        case ConstraintAction::Equal:
            definition.type = ConstraintType::Equal;
            break;
        case ConstraintAction::Fix:
            definition.type = ConstraintType::Fix;
            requiredRefs = 1;
            break;
        case ConstraintAction::Dimension:
            definition.type = ConstraintType::Distance;
            break;
        case ConstraintAction::Angle:
            definition.type = ConstraintType::Angle;
            break;
        default:
            return {State::InvalidSelection, std::nullopt};
    }

    const auto capabilities = sketch_.capabilities();
    if (request.action == ConstraintAction::Dimension) {
        if (!capabilities.supports(ConstraintType::Length) && !capabilities.supports(ConstraintType::Distance)) {
            return {State::Unsupported, std::nullopt};
        }
    } else if (!capabilities.supports(definition.type)) {
        return {State::Unsupported, std::nullopt};
    }
    if (refs.empty()) {
        return {State::NeedsMoreInput, std::nullopt};
    }

    std::array<RefKind, 2> kinds{RefKind::Invalid, RefKind::Invalid};
    for (size_t i = 0; i < refs.size(); ++i) {
        const auto entity = sketch_.entity(refs[i].entity);
        if (!entity) {
            return {State::InvalidSelection, std::nullopt};
        }
        kinds[i] = classifyRef(entity.value(), refs[i].sub);
        if (kinds[i] == RefKind::Invalid) {
            return {State::InvalidSelection, std::nullopt};
        }
    }

    if (request.action == ConstraintAction::Dimension && kinds[0] == RefKind::Line) {
        definition.type = ConstraintType::Length;
        requiredRefs = 1;
        if (*request.value == 0) {
            return {State::InvalidSelection, std::nullopt};
        }
    }
    if (refs.size() > requiredRefs) {
        return {State::InvalidSelection, std::nullopt};
    }

    // Check partial selections here; Core validates the complete definition below.
    bool compatible = false;
    switch (definition.type) {
        case ConstraintType::Coincident:
        case ConstraintType::Distance:
        case ConstraintType::Fix:
            compatible = kinds[0] == RefKind::Point && (refs.size() == 1 || kinds[1] == RefKind::Point);
            break;
        case ConstraintType::Horizontal:
        case ConstraintType::Vertical:
        case ConstraintType::Length:
        case ConstraintType::Parallel:
        case ConstraintType::Perpendicular:
        case ConstraintType::Angle:
            compatible = kinds[0] == RefKind::Line && (refs.size() == 1 || kinds[1] == RefKind::Line);
            break;
        case ConstraintType::Equal:
            compatible = (kinds[0] == RefKind::Line || kinds[0] == RefKind::Circular) && (refs.size() == 1 || kinds[1] == kinds[0]);
            break;
        case ConstraintType::Tangent:
            compatible = (kinds[0] == RefKind::Line || kinds[0] == RefKind::Circular) &&
                         (refs.size() == 1 || (kinds[1] == RefKind::Circular || (kinds[0] == RefKind::Circular && kinds[1] == RefKind::Line)));
            break;
        default:
            break;
    }
    if (!compatible) {
        return {State::InvalidSelection, std::nullopt};
    }
    if (!capabilities.supports(definition.type)) {
        return {State::Unsupported, std::nullopt};
    }
    if (refs.size() < requiredRefs) {
        return {State::NeedsMoreInput, std::nullopt};
    }

    definition.refs.assign(refs.begin(), refs.end());
    if (definition.type == ConstraintType::Fix) {
        const auto position = sketch_.pointPosition(refs[0]);
        if (!position) {
            return {State::InvalidSelection, std::nullopt};
        }
        definition.fixedPosition = position.value();
    }
    const auto supported = sketch_.supportsConstraint(definition);
    if (!supported) {
        return {supported.error().code == core::sketch::ErrorCode::Unsupported ? State::Unsupported : State::InvalidSelection, std::nullopt};
    }
    return {State::Ready, std::move(definition)};
}

bool ConstraintActions::apply(const core::sketch::ConstraintDefinition& definition) {
    if (!sketch_.supportsConstraint(definition)) {
        return false;
    }
    const auto added = definition.type == core::sketch::ConstraintType::Coincident
                           ? sketch_.addCoincident(definition.refs[0], definition.refs[1], core::sketch::CoincidentPlacement::Midpoint)
                           : sketch_.addConstraint(definition);
    if (!added) {
        return false;
    }
    const auto solved = sketch_.solve();
    return solved && solved.value().status == core::sketch::SolveStatus::Converged;
}
