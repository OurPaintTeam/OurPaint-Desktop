#ifndef OURPAINT_RENDERSTYLE_H
#define OURPAINT_RENDERSTYLE_H

#include <QPointF>
#include "Colors.h"

/// --- Base components ---

struct GlowStyle {
    Color color = Color::Blue;
    quint16 size = 5;
    bool activity  = false;

    static GlowStyle createDefault() { return {}; }
};

struct TextLabelStyle {
    QPointF offset = {0, 0};
    Color color = Color::Black;
    quint16 size = 1;

    static TextLabelStyle createDefault() { return {}; }
};

struct IDLabelStyle {
    //QPointF position = {0, 0};
    TextLabelStyle label;
    bool activity = false;

    static IDLabelStyle createDefault() { return {}; }
};

struct ObjectStyle {
    Color color = Color::Black;
    quint16 size = 1;

    static ObjectStyle createDefault() { return {}; }
};

/// --- Figures ---

struct FigureStyle {
    ObjectStyle object;
    GlowStyle glow;
    IDLabelStyle id;

    static FigureStyle createDefault() {
        return {ObjectStyle::createDefault(), GlowStyle::createDefault(), IDLabelStyle::createDefault()};
    }
};

struct PointStyle {
    FigureStyle figure;
    quint16 pointRadius = 1;

    static PointStyle createDefault() {
        return {FigureStyle::createDefault(), 1};
    }
};

struct LineLengthLabel {
    TextLabelStyle label;

    static LineLengthLabel createDefault() { return {}; }
};

struct LineStyle {
    FigureStyle figure;
    LineLengthLabel lengthLabel;

    static LineStyle createDefault() {
        return {FigureStyle::createDefault(), LineLengthLabel::createDefault()};
    }
};

struct RadiusLabel {
    TextLabelStyle text;
    Color lineColor = Color::LightGray;

    static RadiusLabel createDefault() {
        return {TextLabelStyle::createDefault(), Color::LightGray};
    }
};

struct CircleStyle {
    FigureStyle figure;
    RadiusLabel radiusLabel;

    static CircleStyle createDefault() {
        return {FigureStyle::createDefault(), RadiusLabel::createDefault()};
    }
};

struct ArcHeightLabel {
    TextLabelStyle heightText;
    Color lineColor = Color::LightGray;

    static ArcHeightLabel createDefault() {
        return {TextLabelStyle::createDefault(), Color::LightGray};
    }
};

struct ArcStyle {
    FigureStyle figure;
    RadiusLabel radiusLabel;
    ArcHeightLabel heightLabel;

    static ArcStyle createDefault() {
        return {FigureStyle::createDefault(), RadiusLabel::createDefault(), ArcHeightLabel::createDefault()};
    }
};

/// --- Requirements ---

enum class RequirementVisualType {
    None,
    ArrowLine,
    ArcArrow,
    BoxMark
};

struct RequirementsStyle {
    ObjectStyle object;
    GlowStyle glow;
    TextLabelStyle label;
    RequirementVisualType visualType = RequirementVisualType::None;

    static RequirementsStyle createDefault() {
        return {ObjectStyle::createDefault(), GlowStyle::createDefault(),
                TextLabelStyle::createDefault(), RequirementVisualType::None};
    }
};

#endif // OURPAINT_RENDERSTYLE_H
