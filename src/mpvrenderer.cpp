#include "mpvrenderer.h"
#include "mpvobject.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QDebug>
#include <QSGRendererInterface>

static void *get_proc_address(void *ctx, const char *name)
{
    Q_UNUSED(ctx)
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) {
        return nullptr;
    }
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

MpvRenderer::MpvRenderer(MpvObject *mpvObject)
    : m_mpvObject(mpvObject)
{
}

MpvRenderer::~MpvRenderer()
{
    // Note: The render context is owned by MpvObject
}

void MpvRenderer::initializeRenderContext()
{
    if (m_initialized || !m_mpvObject || !m_mpvObject->mpvHandle()) {
        return;
    }

    mpv_handle *mpv = m_mpvObject->mpvHandle();

    // Set up OpenGL render parameters
    mpv_opengl_init_params gl_init_params{
        .get_proc_address = get_proc_address,
        .get_proc_address_ctx = nullptr,
    };

    // Static variable for advanced control flag
    static int advancedControl = 1;

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advancedControl},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    int result = mpv_render_context_create(&m_renderCtx, mpv, params);
    if (result < 0) {
        qWarning() << "Failed to create mpv render context:" << mpv_error_string(result);
        return;
    }

    // Set up update callback
    mpv_render_context_set_update_callback(m_renderCtx, [](void *ctx) {
        MpvRenderer *self = static_cast<MpvRenderer *>(ctx);
        if (self->m_mpvObject) {
            // Trigger update on the MpvObject itself
            QMetaObject::invokeMethod(self->m_mpvObject, "update", Qt::QueuedConnection);
        }
    }, this);

    m_initialized = true;
    qDebug() << "mpv render context initialized successfully";
}

QOpenGLFramebufferObject *MpvRenderer::createFramebufferObject(const QSize &size)
{
    m_size = size;

    // Initialize render context on first FBO creation (when GL context is ready)
    if (!m_initialized) {
        initializeRenderContext();
    }

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    return new QOpenGLFramebufferObject(size, format);
}

void MpvRenderer::synchronize(QQuickFramebufferObject *item)
{
    m_mpvObject = static_cast<MpvObject *>(item);
}

void MpvRenderer::render()
{
    if (!m_renderCtx) {
        qDebug() << "render() called but no render context";
        return;
    }

    if (!m_mpvObject) {
        qDebug() << "render() called but no mpv object";
        return;
    }

    QOpenGLFramebufferObject *fbo = framebufferObject();
    if (!fbo) {
        qDebug() << "render() called but no FBO";
        return;
    }

    // Check if there's a new frame to render
    uint64_t flags = mpv_render_context_update(m_renderCtx);

    if (!(flags & MPV_RENDER_UPDATE_FRAME)) {
        // No new frame, skip rendering
        return;
    }

    // Get device pixel ratio for HiDPI support
    qreal dpr = 1.0;
    if (m_mpvObject->window()) {
        dpr = m_mpvObject->window()->devicePixelRatio();
    }

    mpv_opengl_fbo mpfbo{
        .fbo = static_cast<int>(fbo->handle()),
        .w = static_cast<int>(m_size.width() * dpr),
        .h = static_cast<int>(m_size.height() * dpr),
        .internal_format = 0  // Let mpv decide
    };

    int flip_y = 0;  // Qt FBOs don't need flipping in Qt 6

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };

    // Render the frame
    mpv_render_context_render(m_renderCtx, params);
}
