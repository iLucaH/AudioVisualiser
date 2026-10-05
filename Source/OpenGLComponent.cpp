/*
  ==============================================================================

    OpenGLComponent.cpp
    Created: 18 Nov 2025 10:53:16pm
    Author:  lucas

  ==============================================================================
*/

#include <JuceHeader.h>
#include "OpenGLComponent.h"
#include "RenderHeaders.h"
#include "PostProcessEffects.h"

//==============================================================================
OpenGLComponent::OpenGLComponent(AudioVisualiserAudioProcessor &p, ApplicationSettings& appSettings) : processor(p), appSettings(appSettings), ringBuffer(p.getRingBuffer()), readBuffer(2, RING_BUFFER_READ_SIZE), postProcessor(*this) {
    addRenderState(std::make_unique<Classic1_2D>(1, openGLContext));
    addRenderState(std::make_unique<Classic2_2D>(2, openGLContext));
    addRenderState(std::make_unique<Classic3_2D>(3, openGLContext));
    addRenderState(std::make_unique<Classic4_2D>(4, openGLContext));
    addRenderState(std::make_unique<TimeDomain1_2D>(5, openGLContext));
    addRenderState(std::make_unique<TimeDomain2_2D>(6, openGLContext));
    addRenderState(std::make_unique<TimeDomain3_2D>(7, openGLContext));
    addRenderState(std::make_unique<SDF_1_2D>(8, openGLContext));
    addRenderState(std::make_unique<AskAI>(9, *this, openGLContext, appSettings));

    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createWaveDistortionPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createAberrationPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createVignettePostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createPixelatePostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createInvertPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createColourGradingPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createColourTintPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createBlurPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createSharpenPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createFilmGrainPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createScanlinesPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createPosterizePostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createEdgeDetectPostProcessingEffect(), openGLContext));
    postProcessor.addPostProcessEffect(std::make_unique<PostProcessEffect>(createBarrelDistortionPostProcessingEffect(), openGLContext));
    
    setOpaque(true); // Indicates that no part of this Component is transparent
    openGLContext.setRenderer(this); // Set this instance as the renderer for the context
    openGLContext.setContinuousRepainting(true); // Tell the context to repaint on a loop
    openGLContext.attachTo(*this); // Finally - we attach the context to this Component.
    juce::Desktop::getInstance().addGlobalMouseListener(this);
}

OpenGLComponent::~OpenGLComponent() {
    openGLContext.detach();
}

void OpenGLComponent::mouseUp(const juce::MouseEvent & event) {
    if (fullScreenMode.load() && event.mods.isShiftDown()) {
        setFullScreen(false);
        juce::MessageManager::callAsync([this]() {
            appSettings.getEventBus().emit(Send_Events::SettingsUpdated);
        });
    }
}

void OpenGLComponent::paint(juce::Graphics& g) {
    // Everything that is drawn here overlays the OpenGL render.
}

void OpenGLComponent::resized() {
}

void OpenGLComponent::newOpenGLContextCreated() {
    DBG("New OpenGL Context is being created.");
    juce::gl::glDebugMessageControl(juce::gl::GL_DONT_CARE, juce::gl::GL_DONT_CARE, juce::gl::GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, juce::gl::GL_FALSE);
    for (int i = 0; i < renderStates.size(); i++) {
        RenderState* renderState = renderStates[i].get();
        if (renderState->isInititalised() == false) {
            renderState->initAndCompileShaders();
        }
    }

    // Video encoder initialised in this function because it creates an GLTexture which we need to use for the render target.
    videoEncoderWidth.store(getWidth());
    videoEncoderHeight.store(getHeight());
    DBG("Loading Video Encoder Sizing:");
    DBG("getWidth(): " << getWidth());
    DBG("getHeight(): " << getHeight());
    DBG("videoEncoderWidth: " << (int) videoEncoderWidth.load());
    DBG("videoEncoderHeight: " << (int) videoEncoderHeight.load());
    videoEncoder = std::make_unique<VideoEncoder>((int) videoEncoderWidth.load(), (int) videoEncoderHeight.load());
    
    juce::gl::glGenFramebuffers(1, &fbo);
    juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, fbo);
    juce::gl::glFramebufferTexture2D(juce::gl::GL_FRAMEBUFFER, juce::gl::GL_COLOR_ATTACHMENT0, juce::gl::GL_TEXTURE_2D, videoEncoder->getTextureID(), 0);
    juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);

    if (juce::gl::glCheckFramebufferStatus(juce::gl::GL_FRAMEBUFFER) != juce::gl::GL_FRAMEBUFFER_COMPLETE) {
        DBG("FBO creation incomplete!");
    }

    // Init the post processor, and let it subscribe to any events here now that selectorTabPanel has been initialised.
    const auto scale = openGLContext.getRenderingScale();
    postProcessor.init(juce::roundToInt(getWidth() * scale), juce::roundToInt(getHeight() * scale), appSettings.getEventBus());
}

void OpenGLComponent::renderOpenGL() {
    time++;
    juce::OpenGLHelpers::clear(juce::Colours::black);
    unsigned int currentState = selectedState.load();
    if (currentState < 1 || currentState > renderStates.size())
        return;
    RenderState* renderState = renderStates[currentState - 1].get();
    if (renderState->isInititalised() == false) {
        DBG("Shader not initialised!");
        return;
    }
    GLuint progID = renderState->getShaderProgramID();
    if (progID == -1) {
        DBG("Shader Program ID is invalid!");
        return;
    }
    openGLContext.extensions.glUseProgram(progID);

    GLuint timeUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "time");
    openGLContext.extensions.glUniform1i(timeUniform, time);

    GLuint leftRMSUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "leftRMS");
    openGLContext.extensions.glUniform1f(leftRMSUniform, processor.getRMS(0) * appSettings.getAudioScalar());
    GLuint rightRMSUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "rightRMS");
    openGLContext.extensions.glUniform1f(rightRMSUniform, processor.getRMS(1) * appSettings.getAudioScalar());

    GLuint screenWidthUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "screenWidth");
    GLuint screenHeightUniform = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "screenHeight");
    auto scale = (float) openGLContext.getRenderingScale();
    const int screenWidth = juce::roundToInt(getWidth() * scale);
    const int screenHeight = juce::roundToInt(getHeight() * scale);
    openGLContext.extensions.glUniform1f(screenWidthUniform, (float)screenWidth);
    openGLContext.extensions.glUniform1f(screenHeightUniform, (float)screenHeight);

    ringBuffer.readSamples(readBuffer, RING_BUFFER_READ_SIZE);
    juce::FloatVectorOperations::clear(visualizationBufferTD, RING_BUFFER_READ_SIZE);
    for (int i = 0; i < 2; ++i) { // Sum channels together
        juce::FloatVectorOperations::add(visualizationBufferTD, readBuffer.getReadPointer(i, 0), RING_BUFFER_READ_SIZE);
    }
    juce::FloatVectorOperations::multiply(visualizationBufferTD, appSettings.getAudioScalar(), RING_BUFFER_READ_SIZE);
    GLuint visualizationUniformTD = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "audioBufferTD");
    openGLContext.extensions.glUniform1fv(visualizationUniformTD, RING_BUFFER_READ_SIZE, visualizationBufferTD);

    GLuint visualizationUniformFD = openGLContext.extensions.glGetUniformLocation(renderState->getShaderProgramID(), "audioBufferFD");
    openGLContext.extensions.glUniform1fv(visualizationUniformFD, FFT_BIN_SIZE, processor.getShaderFFT().data());

    if (!isOpenGLEnabled()) { // If we aren't rendering openGL, then dont let it reach here.
        return;
    }

    // Video Encoding
    juce::String* filePtr = pendingEncoderFileName.exchange(nullptr);
    if (filePtr) {
        videoEncoder->startRecordingSession(*filePtr);
        delete filePtr; // filePtr is created using new
    }
    if (pendingStop.exchange(false)) {
        videoEncoder->finishRecordingSession();
        if (videoEncoder->getWidth() != (int) videoEncoderWidth.load() || videoEncoder->getHeight() != (int) videoEncoderHeight.load()) {
            DBG("Resetting video encoder now!");
            videoEncoder.reset(); // Calls delete on the old videoEncoder object.
            videoEncoder = std::make_unique<VideoEncoder>((int) videoEncoderWidth.load(), (int) videoEncoderHeight.load()); // Create the new videoEncoder object.
            DBG("New encoder initialised!");

            // Handle removing the old frame buffer object with the old texture attached and then creating a new one with the correct texture ID.
            juce::gl::glDeleteFramebuffers(1, &fbo);
            juce::gl::glGenFramebuffers(1, &fbo);
            juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, fbo);
            juce::gl::glFramebufferTexture2D(juce::gl::GL_FRAMEBUFFER, juce::gl::GL_COLOR_ATTACHMENT0, juce::gl::GL_TEXTURE_2D, videoEncoder->getTextureID(), 0);
            juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0); // Always good to unbind the frame buffer even though we are just going to bind it again straight away anyway.
            DBG("Frame buffer has been re-created!");
        }
    }
    if (videoEncoder->isActive()) {
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, fbo);
        juce::gl::glViewport(0, 0, (int) videoEncoderWidth.load(), (int) videoEncoderHeight.load());
        renderState->render();
        videoEncoder->addVideoFrame();
    }

    postProcessor.updateRenderOrder(screenWidth, screenHeight);
    bool postProcessingEnabled = !postProcessor.noPostProcessorsEnabled() && postProcessor.isEnabledGlobal();
    if (postProcessingEnabled) {
        // If we want to do screen space effects,
        // then render the screen to a texture first.
        GLuint frameBuffer = postProcessor.peek()->getScreenSpaceQuadFrameBuffer(); // We need to get the frame buffer from the screen space effect.
        
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, frameBuffer);
    } else {
        // We are not doing any post processing so we can render to the screen.
        juce::gl::glBindFramebuffer(juce::gl::GL_FRAMEBUFFER, 0);
    }

    juce::gl::glViewport(0, 0, screenWidth, screenHeight);
    if (postProcessingEnabled)
        juce::gl::glClear(juce::gl::GL_COLOR_BUFFER_BIT);   // see note below
    renderState->render();

    if (postProcessingEnabled)
        postProcessor.renderAll(screenWidth, screenHeight);
}

void OpenGLComponent::resetVideoRecorder(int width, int height) {
    videoEncoderWidth.store(width);
    videoEncoderHeight.store(height);
    // Tell the render loop on the OpenGL Thread that the recording should be stopped.
    // After each time the recording is stopped, it will check if the sizing has changed and if so, reset the encoder.
    // The VideoEncoder object needs to be handled on the GLThread because it deals with a GL Texture Reference.
    DBG("Resetting video encoder call has been made!");
    pendingStop.store(true);
}

void OpenGLComponent::resizeComponent(juce::Rectangle<int> visualiserArea) {
    postProcessor.resizeTargets(visualiserArea);
}

void OpenGLComponent::openGLContextClosing() {
}