#ifndef CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_

#include "base/compiler_specific.h"
#include "ui/views/view.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/gfx/geometry/size.h"

class BrowserView;
class ContentsWebView;

namespace views {
    class MillixBarView : public View {
        public:
            explicit MillixBarView(BrowserView* browser_view);
            ~MillixBarView() override;
            gfx::Size CalculatePreferredSize() const override;
            void SetWebContents(std::unique_ptr<content::WebContents> new_contents);
            void SetViewBounds(int x, int y, int width, int height);
            content::WebContents* GetWebContents();
        protected:
            void OnThemeChanged() override;
        private:
            // The parent of this view. Not owned.
            BrowserView* browser_view_;
            std::unique_ptr<content::WebContents> web_contents_;
            ContentsWebView* contents_webview_;
    };
}

#endif  // CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_