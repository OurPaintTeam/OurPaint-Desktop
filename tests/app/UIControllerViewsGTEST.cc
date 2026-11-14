#include <gtest/gtest.h>

#include "IViewportController.h"
#include "UIController.h"

TEST(UIControllerViews, IndependentViewsSurviveClosingAndRenaming) {
    UIController ui;
    const DocumentId document = ui.createFile("Sketch");
    ASSERT_NE(document, InvalidDocumentId);
    EXPECT_EQ(ui.activeView(), InvalidViewId);

    const auto first = ui.openView(document);
    const auto second = ui.openView(document);
    ASSERT_NE(first, second);
    auto* survivingViewport = ui.findViewport(second);
    ASSERT_NE(survivingViewport, nullptr);
    ASSERT_NE(ui.findViewport(first), nullptr);
    EXPECT_NE(ui.findViewport(first), survivingViewport);
    EXPECT_EQ(ui.documents().size(), 1u);

    ui.selectTool(first, ToolId::Point);
    EXPECT_EQ(ui.activeToolId(first), ToolId::Point);
    EXPECT_EQ(ui.activeToolId(second), ToolId::Cursor);
    ui.setActiveView(first);
    ASSERT_TRUE(ui.renameDocument(document, "Plate"));
    EXPECT_EQ(ui.documentOf(second), document);
    ui.closeView(first);
    EXPECT_EQ(ui.findViewport(first), nullptr);
    EXPECT_EQ(ui.findViewport(second), survivingViewport);
    EXPECT_EQ(ui.activeView(), second);
    ui.closeView(second);
    const auto reopened = ui.openView(document);
    EXPECT_NE(reopened, first);
    EXPECT_NE(reopened, second);
    EXPECT_EQ(ui.documentOf(reopened), document);
    EXPECT_EQ(ui.documentName(document), "Plate");
    EXPECT_EQ(ui.openView(InvalidDocumentId), InvalidViewId);
}

TEST(UIControllerViews, ObserverCanDetachWhileViewportStillExists) {
    struct Observer final : IUIObserver {
        explicit Observer(UIController& owner) : owner(owner) {}
        void viewAboutToClose(ViewId view) override {
            closing.push_back(view);
            EXPECT_NE(owner.findViewport(view), nullptr);
        }
        void activeViewChanged(ViewId) override {}
        void toolChanged(ViewId view, ToolId) override { changed = view; }
        void redrawRequested(ViewId) override {}
        UIController& owner;
        ViewId changed = InvalidViewId;
        std::vector<ViewId> closing;
    };

    UIController ui;
    Observer observer(ui);
    ui.setObserver(&observer);
    const auto document = ui.createFile("Sketch");
    const auto first = ui.openView(document);
    const auto second = ui.openView(document);
    ui.selectTool(second, ToolId::Line);
    EXPECT_EQ(observer.changed, second);
    ui.closeView(first);
    ui.closeView(first);
    ui.clearViews();
    EXPECT_EQ(observer.closing, (std::vector<ViewId>{first, second}));
    ui.setObserver(nullptr);
}
