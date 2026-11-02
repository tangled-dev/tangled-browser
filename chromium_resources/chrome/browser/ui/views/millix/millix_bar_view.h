#ifndef CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/gfx/geometry/size.h"

class BrowserView;
class ContentsWebView;

namespace content {
class WebContents;
}

namespace views {
    class MillixBarView : public View, public gfx::AnimationDelegate {
        METADATA_HEADER(MillixBarView, View)
        public:
            static constexpr int kDefaultHeight = 50;

            explicit MillixBarView(BrowserView* browser_view);
            ~MillixBarView() override;
            void SetWebContents(std::unique_ptr<content::WebContents> new_contents);
            void SetViewBounds(int x, int y, int width, int height);
            content::WebContents* GetWebContents();
            // Animates the bar to |height| DIPs. The browser layout follows the
            // preferred size, so the page content moves with the bar.
            void SetBarHeight(int height);
            void Layout(PassKey) override;
            gfx::Size CalculatePreferredSize(
                const SizeBounds& available_size) const override;
        protected:
            void OnThemeChanged() override;
            // gfx::AnimationDelegate:
            void AnimationProgressed(const gfx::Animation* animation) override;
            void AnimationEnded(const gfx::Animation* animation) override;
        private:
            int GetCurrentHeight() const;

            // The parent of this view. Not owned.
            raw_ptr<BrowserView> browser_view_;
            std::unique_ptr<content::WebContents> web_contents_;
            raw_ptr<ContentsWebView> contents_webview_;
            // The bar animates from |start_height_| to |target_height_|.
            int start_height_ = kDefaultHeight;
            int target_height_ = kDefaultHeight;
            gfx::SlideAnimation height_animation_{this};
    };
}

#endif  // CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_
