#ifndef CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/gfx/geometry/size.h"

class BrowserView;
class ContentsWebView;

namespace content {
class WebContents;
}

namespace views {
    class MillixBarView : public View {
        METADATA_HEADER(MillixBarView, View)
        public:
            explicit MillixBarView(BrowserView* browser_view);
            ~MillixBarView() override;
            void SetWebContents(std::unique_ptr<content::WebContents> new_contents);
            void SetViewBounds(int x, int y, int width, int height);
            content::WebContents* GetWebContents();
            void Layout(PassKey) override;
        protected:
            void OnThemeChanged() override;
        private:
            // The parent of this view. Not owned.
            raw_ptr<BrowserView> browser_view_;
            std::unique_ptr<content::WebContents> web_contents_;
            raw_ptr<ContentsWebView> contents_webview_;
    };
}

#endif  // CHROME_BROWSER_UI_VIEWS_MILLIX_MILLIX_BAR_VIEW_H_
