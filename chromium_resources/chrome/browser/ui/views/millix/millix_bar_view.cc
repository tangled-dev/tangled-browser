#include "chrome/browser/ui/views/millix/millix_bar_view.h"

#include "base/time/time.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/frame/contents_web_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/animation/tween.h"
#include "ui/gfx/geometry/size.h"
#include "ui/native_theme/native_theme.h"
#include "ui/views/view.h"

namespace views {
    MillixBarView::MillixBarView(BrowserView* browser_view) : browser_view_(browser_view) {
        contents_webview_ = AddChildView(std::make_unique<ContentsWebView>(browser_view->GetProfile()));
        contents_webview_->set_is_primary_web_contents_for_window(true);
        // Zero when animations are disabled, so the bar jumps to its height.
        height_animation_.SetSlideDuration(
            gfx::Animation::RichAnimationDuration(base::Milliseconds(200)));
        height_animation_.SetTweenType(gfx::Tween::EASE_OUT);
    }

    MillixBarView::~MillixBarView() {
        // Detach before |web_contents_| is destroyed.
        contents_webview_->SetWebContents(nullptr);
    }

    void MillixBarView::SetWebContents(std::unique_ptr<content::WebContents> new_contents) {
        web_contents_ = std::move(new_contents);
        contents_webview_->SetWebContents(web_contents_.get());
    }

    content::WebContents* MillixBarView::GetWebContents(){
        return web_contents_.get();
    }

    void MillixBarView::SetViewBounds(int x, int y, int width, int height) {
        SetBounds(x, y, width, height);
    }

    void MillixBarView::SetBarHeight(int height) {
        if (height == target_height_) {
            return;
        }
        // Start from where the bar is now, which may be mid-animation. Set
        // both heights before Reset(), which can notify the delegate.
        start_height_ = GetCurrentHeight();
        target_height_ = height;
        height_animation_.Reset(0);
        height_animation_.Show();
    }

    int MillixBarView::GetCurrentHeight() const {
        return gfx::Tween::IntValueBetween(height_animation_.GetCurrentValue(),
                                           start_height_, target_height_);
    }

    void MillixBarView::Layout(PassKey) {
        contents_webview_->SetBounds(0, 0, width(), height());
    }

    gfx::Size MillixBarView::CalculatePreferredSize(
        const SizeBounds& available_size) const {
        return gfx::Size(0, GetCurrentHeight());
    }

    void MillixBarView::AnimationProgressed(const gfx::Animation* animation) {
        PreferredSizeChanged();
    }

    void MillixBarView::AnimationEnded(const gfx::Animation* animation) {
        PreferredSizeChanged();
    }

    void MillixBarView::OnThemeChanged() {
        View::OnThemeChanged();
        if (!web_contents_ || !web_contents_->GetWebUI()) {
            return;
        }
        base::DictValue is_dark_theme;
        is_dark_theme.Set("is_dark_theme",
                          ui::NativeTheme::GetInstanceForWeb()->preferred_color_scheme() ==
                              ui::NativeTheme::PreferredColorScheme::kDark);
        web_contents_->GetWebUI()->CallJavascriptFunctionUnsafe(
            "cr.webUIListenerCallback", base::Value("onThemeChanged"),
            is_dark_theme);
    }

    BEGIN_METADATA(MillixBarView)
    END_METADATA
}
