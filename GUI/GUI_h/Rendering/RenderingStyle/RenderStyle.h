#ifndef OURPAINT_RENDERSTYLE_H
#define OURPAINT_RENDERSTYLE_H

#include <QPointF>
#include <QPair>
#include <QLineEdit>
#include <QPointer>

#include "Colors.h"
#include "AngleLineEdit.h"

/// --- Base components ---

struct GlowStyle {
    Color color = Color::Blue;
    quint16 size = 5;
    bool activity  = false;
};


struct Position{
    QPointF lt;
    QPointF rt;
    QPointF lb;
    QPointF rb;
};


struct TextLabelStyle {
    QPointF offset = {0, 0};
    Color color = Color::Black;
    quint16 size = 1;
    Position position;
    QPointer<AngleLineEdit> textEdit = nullptr;
    QRect prevRect;

    bool editing = false;
    bool active = true;
};


struct IDLabelStyle {
    //QPointF position = {0, 0};
    TextLabelStyle label;
    bool activity = false;
};


struct ObjectStyle {
    Color color = Color::Black;
    quint16 size = 1;
};


/// --- Figures ---

struct FigureStyle {
    ObjectStyle object;
    GlowStyle glow;
    IDLabelStyle id;
};


struct PointStyle {
    FigureStyle figure;
    quint16 pointRadius = 1;

     void createNormalPointStyle() {
        figure.object.size = 1;
        pointRadius = 1;
        figure.glow.activity = false;
        figure.id.activity = false;
    }

    void createSelectedPointStyle() {
         figure.object.size = 1;
         figure.object.color = Color::Black;
         pointRadius = 2;
         figure.glow.color = Color::Blue;
         figure.glow.size = 5;
         figure.glow.activity = true;
         figure.id.activity = true;
     }

};


struct LineLengthLabel {
    TextLabelStyle label;
};


struct ServiceLine{
    FigureStyle lineStyle;
    QLineF position;
    bool active = false;
    QPointF offset{0,0};
};


struct LineStyle {
    FigureStyle figure;
    LineLengthLabel lengthLabel;
    ServiceLine serviceLine;

    void createNormalLineStyle() {
        figure.object.size = 1;
        figure.glow.activity = false;
        figure.id.activity = false;
    }

    void createSelectedLineStyle() {
        figure.object.color = Color::Black;
        figure.object.size = 2;
        figure.glow.color = Color::Blue;
        figure.glow.size = 5;
        figure.glow.activity = true;
        figure.id.activity = true;
    }
};


struct RadiusLabel {
    TextLabelStyle text;
    [[maybe_unused]] Color lineColor = Color::LightGray;
    [[maybe_unused]] bool activity = false;

};


struct CircleStyle {
    FigureStyle figure;
    RadiusLabel radiusLabel;

    void createNormalCircleStyle() {
        figure.object.size = 1;
        figure.glow.activity = false;
        figure.id.activity = false;
    }

    void createSelectedCircleStyle() {
        figure.object.color = Color::Black;
        figure.object.size = 2;
        figure.glow.color = Color::Blue;
        figure.glow.size = 5;
        figure.glow.activity = true;
        figure.id.activity = true;
    }

};


struct ArcHeightLabel {
    TextLabelStyle heightText;
    [[maybe_unused]] Color lineColor = Color::LightGray;
};


struct ArcStyle {
    FigureStyle figure;
    RadiusLabel radiusLabel;
    ArcHeightLabel heightLabel;

    void createNormalArcStyle() {
        figure.object.size = 1;
        figure.glow.activity = false;
        figure.id.activity = false;
    }

    void createSelectedArcStyle() {
        figure.object.color = Color::Black;
        figure.object.size = 2;
        figure.glow.color = Color::Blue;
        figure.glow.size = 5;
        figure.glow.activity = true;
        figure.id.activity = true;
    }
};

/// --- Requirements ---

enum class RequirementVisualType {
    None,
    ArrowLine [[maybe_unused]],
    ArcArrow [[maybe_unused]],
    BoxMark [[maybe_unused]]
};


struct [[maybe_unused]] RequirementsStyle {
    ObjectStyle object;
    [[maybe_unused]] GlowStyle glow;
    TextLabelStyle label;
    [[maybe_unused]] RequirementVisualType visualType = RequirementVisualType::None;
};

#endif // OURPAINT_RENDERSTYLE_H
