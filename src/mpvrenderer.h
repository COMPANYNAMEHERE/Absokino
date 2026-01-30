#ifndef MPVRENDERER_H
#define MPVRENDERER_H

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <mpv/client.h>
#include <mpv/render_gl.h>

class MpvObject;

/**
 * @brief MpvRenderer - Renderer that draws mpv frames into Qt's FBO
 *
 * This renderer uses mpv's render API to draw video frames
 * into a Qt framebuffer object, which is then composited by Qt Quick.
 */
class MpvRenderer : public QQuickFramebufferObject::Renderer
{
public:
    explicit MpvRenderer(MpvObject *mpvObject);
    ~MpvRenderer() override;

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void render() override;
    void synchronize(QQuickFramebufferObject *item) override;

private:
    void initializeRenderContext();

    MpvObject *m_mpvObject = nullptr;
    mpv_render_context *m_renderCtx = nullptr;
    QSize m_size;
    bool m_initialized = false;
    bool m_forceRender = true;
};

#endif // MPVRENDERER_H
