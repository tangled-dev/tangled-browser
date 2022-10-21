#include "chrome/browser/ui/views/millix/millix_bar_view.h"
#include "ui/views/view.h"
#include "ui/gfx/geometry/size.h"
#include "ui/native_theme/native_theme.h"
#include "chrome/browser/ui/views/frame/contents_web_view.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "content/public/browser/web_contents.h"

namespace views {
    MillixBarView::MillixBarView(BrowserView* browser_view) : browser_view_(browser_view) {
        contents_webview_ = AddChildView(std::make_unique<ContentsWebView>(browser_view->GetProfile()));    
        contents_webview_->set_is_primary_web_contents_for_window(true);
        this->SetPreferredSize(gfx::Size(0, 50));
    }

    MillixBarView::~MillixBarView() {

    }

    void MillixBarView::SetWebContents(std::unique_ptr<content::WebContents> new_contents) {
        web_contents_ = std::move(new_contents);
        contents_webview_->SetWebContents(web_contents_.get());
    }

    content::WebContents* MillixBarView::GetWebContents(){
        return web_contents_.get();
    }

    gfx::Size MillixBarView::CalculatePreferredSize() const {
        if(!browser_view_){
            return gfx::Size();
        }
        return this->GetPreferredSize();
    }

    void MillixBarView::SetViewBounds(int x, int y, int width, int height) {
        View::SetBounds(x, y, width, height);
        contents_webview_->SetBounds(0, 0, width, height);
    }

    void MillixBarView::OnThemeChanged() {
        View::OnThemeChanged();
        base::DictionaryValue isDarkTheme;
        isDarkTheme.SetBoolean("is_dark_theme", ui::NativeTheme::GetInstanceForWeb()->ShouldUseDarkColors());
        web_contents_->GetWebUI()->CallJavascriptFunctionUnsafe("cr.webUIListenerCallback", base::Value("onThemeChanged"), isDarkTheme);
    }

}