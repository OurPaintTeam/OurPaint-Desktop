#include <gtest/gtest.h>

#include <set>

#include "IViewportController.h"
#include "UIController.h"
#include "render/ViewportStyle.h"

using namespace app;

TEST(UIControllerViews, IndependentViewsSurviveClosingAndRenaming) {
    UIController ui;
    const DocumentId document = ui.createDocument("Sketch");
    ASSERT_NE(document, InvalidDocumentId);
    EXPECT_EQ(ui.activeDocumentView(), InvalidViewId);

    const auto first = ui.createDocumentView(document);
    const auto second = ui.createDocumentView(document);
    ASSERT_NE(first, second);
    auto* survivingViewport = ui.findViewport(second);
    ASSERT_NE(survivingViewport, nullptr);
    ASSERT_NE(ui.findViewport(first), nullptr);
    EXPECT_NE(ui.findViewport(first), survivingViewport);
    EXPECT_EQ(ui.documents().size(), 1u);

    ui.selectTool(first, ToolId::Point);
    EXPECT_EQ(ui.activeToolId(first), ToolId::Point);
    EXPECT_EQ(ui.activeToolId(second), ToolId::Cursor);
    ui.setActiveDocumentView(first);
    ASSERT_TRUE(ui.renameDocument(document, "Plate"));
    EXPECT_EQ(ui.documentOf(second), document);
    ui.closeDocumentView(first);
    EXPECT_EQ(ui.findViewport(first), nullptr);
    EXPECT_EQ(ui.findViewport(second), survivingViewport);
    EXPECT_EQ(ui.activeDocumentView(), second);
    ui.closeDocumentView(second);
    const auto reopened = ui.createDocumentView(document);
    EXPECT_NE(reopened, first);
    EXPECT_NE(reopened, second);
    EXPECT_EQ(ui.documentOf(reopened), document);
    EXPECT_EQ(ui.documentName(document), "Plate");
    EXPECT_EQ(ui.createDocumentView(InvalidDocumentId), InvalidViewId);
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
    const auto document = ui.createDocument("Sketch");
    const auto first = ui.createDocumentView(document);
    const auto second = ui.createDocumentView(document);
    ui.selectTool(second, ToolId::Line);
    EXPECT_EQ(observer.changed, second);
    ui.closeDocumentView(first);
    ui.closeDocumentView(first);
    ui.clearDocumentViews();
    EXPECT_EQ(observer.closing, (std::vector<ViewId>{first, second}));
    ui.setObserver(nullptr);
}

TEST(UIControllerViews, ChangingViewportStyleRedrawsEveryViewAndPreservesDocuments) {
    struct Observer final : IUIObserver {
        void viewAboutToClose(ViewId) override {}
        void activeViewChanged(ViewId) override {}
        void toolChanged(ViewId, ToolId) override {}
        void redrawRequested(ViewId view) override { redrawn.insert(view); }
        std::multiset<ViewId> redrawn;
    } observer;

    UIController ui;
    const auto firstDocument = ui.createDocument("Plate");
    const auto secondDocument = ui.createDocument("Shaft");
    const auto first = ui.createDocumentView(firstDocument);
    const auto second = ui.createDocumentView(firstDocument);
    const auto third = ui.createDocumentView(secondDocument);
    ui.setActiveDocumentView(first);
    ui.setObserver(&observer);

    auto style = ui.viewportStyle();
    style.constructionColor = {0.125f, 0.5f, 0.875f, 1.0f};
    ui.setViewportStyle(style);

    EXPECT_FLOAT_EQ(ui.viewportStyle().constructionColor.r, style.constructionColor.r);
    EXPECT_FLOAT_EQ(ui.viewportStyle().constructionColor.g, style.constructionColor.g);
    EXPECT_FLOAT_EQ(ui.viewportStyle().constructionColor.b, style.constructionColor.b);
    EXPECT_EQ(observer.redrawn, (std::multiset<ViewId>{first, second, third}));
    EXPECT_FALSE(ui.documentModified(firstDocument));
    EXPECT_FALSE(ui.documentModified(secondDocument));
    ui.setObserver(nullptr);
}
