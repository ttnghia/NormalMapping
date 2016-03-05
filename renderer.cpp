//------------------------------------------------------------------------------------------
//
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------
#include "renderer.h"

//------------------------------------------------------------------------------------------
Renderer::Renderer(QWidget* _parent):
    QOpenGLWidget(_parent),
    initializedScene(false),
    initializedTestScene(false),
    initializedDepthBuffer(false),
    enabledRenderLight(false),
    currentShadowMode(SHADOW_MAP),
    iboRoom(QOpenGLBuffer::IndexBuffer),
    iboCube(QOpenGLBuffer::IndexBuffer),
    iboSphere(QOpenGLBuffer::IndexBuffer),
    specialKeyPressed(Renderer::NO_KEY),
    mouseButtonPressed(Renderer::NO_BUTTON),
    translation(0.0f, 0.0f, 0.0f),
    translationLag(0.0f, 0.0f, 0.0f),
    rotation(0.0f, 0.0f, 0.0f),
    rotationLag(0.0f, 0.0f, 0.0f),
    zooming(0.0f),
    cubeObject(NULL),
    sphereObject(NULL),
    objLoader(NULL),
    depthTexture(NULL),
    FBODepthMap(NULL),
    cameraPosition(DEFAULT_CAMERA_POSITION),
    cameraFocus(DEFAULT_CAMERA_FOCUS),
    cameraUpDirection(0.0f, 1.0f, 0.0f),
    currentShadingMode(PhongShading),
    currentMeshObject(BUNNY_OBJ),
    currentMouseTransTarget(TRANSFORM_CAMERA),
    currentFloorTexture(DunePattern),
    currentWallTexture(AlienCarving),
    currentMeshObjectTexture(CopperVerdigris),
    ambientLight(0.3)
{
    retinaScale = devicePixelRatio();
    setFocusPolicy(Qt::StrongFocus);

    ////////////////////////////////////////////////////////////////////////////////
    // floor texture
    strListFloorTexture = new QStringList;
    strListFloorTexture->append("DunePattern");
    strListFloorTexture->append("BlueMarblePersian");
    strListFloorTexture->append("BlueMarbleSlabs");
    strListFloorTexture->append("BrownByzantine");
    strListFloorTexture->append("CorrodedTechnoTiles");
    TRUE_OR_DIE(strListFloorTexture->size() == NumFloorTextures,
                "Ohh, you forget to initialize some floor texture...");

    ////////////////////////////////////////////////////////////////////////////////
    // wall texture
    strListWallTexture = new QStringList;
    strListWallTexture->append("AlienCarving");
    strListWallTexture->append("AlternatingBrick");
    strListWallTexture->append("AncientMayanBlocks");
    strListWallTexture->append("BubblyBricks");
    strListWallTexture->append("CarvedSandstone");
    strListWallTexture->append("ChippedBricks");
    strListWallTexture->append("DesertBlushBrick");

    TRUE_OR_DIE(strListWallTexture->size() == NumWallTextures,
                "Ohh, you forget to initialize some wall texture...");

    ////////////////////////////////////////////////////////////////////////////////
    // mesh object texture
    strListMeshObjectTexture = new QStringList;
    strListMeshObjectTexture->append("AluminumTubing");
    strListMeshObjectTexture->append("BronzeAgeArtifact");
    strListMeshObjectTexture->append("CopperVerdigris");
    strListMeshObjectTexture->append("PyramidVerdigris");
    strListMeshObjectTexture->append("TexturedMetal");


    TRUE_OR_DIE(strListMeshObjectTexture->size() == NumMetalTextures,
                "Ohh, you forget to initialize some floor texture...");

}

//------------------------------------------------------------------------------------------
Renderer::~Renderer()
{
}

//------------------------------------------------------------------------------------------
void Renderer::checkOpenGLVersion()
{
    QString verStr = QString((const char*)glGetString(GL_VERSION));
    int major = verStr.left(verStr.indexOf(".")).toInt();
    int minor = verStr.mid(verStr.indexOf(".") + 1, 1).toInt();

    if(!(major >= 4 && minor >= 0))
    {
        QMessageBox::critical(this, "Error",
                              QString("Your OpenGL version is %1.%2, which does not satisfy this program requirement (OpenGL >= 4.0)")
                              .arg(major).arg(minor));
        exit(EXIT_FAILURE);
    }

//    qDebug() << major << minor;
//    qDebug() << verStr;
    //    TRUE_OR_DIE(major >= 4 && minor >= 1, "OpenGL version must >= 4.1");
}

//------------------------------------------------------------------------------------------
GLfloat triangle_vertices[] =
{
    0.0,  0.8, 0,
    -0.8, -0.8, 0,
    0.8, -0.8, 0
};

GLuint vbo = 0;
GLuint vao = 0;
QOpenGLShaderProgram* testProgram = new QOpenGLShaderProgram;

void Renderer::initTestScene()
{

    bool success;

    success = testProgram ->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                    ":/shaders/test.vs.glsl");
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = testProgram ->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                    ":/shaders/test.fs.glsl");
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = testProgram ->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    glUseProgram(testProgram ->programId());

    const char* attribute_name = "v_coord";
    GLint attribute_coord2d = glGetAttribLocation(testProgram->programId(), attribute_name);

    if (attribute_coord2d == -1)
    {
        qDebug() << "Could not bind attribute " << attribute_name;
        return;
    }

    glEnableVertexAttribArray(attribute_coord2d);


    glGenBuffers (1, &vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (float), triangle_vertices, GL_STATIC_DRAW);


    glGenVertexArrays (1, &vao);
//    glBindVertexArray (vao);
    glEnableVertexAttribArray(0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    initializedTestScene = true;
}

//------------------------------------------------------------------------------------------
void Renderer::initScene()
{
    TRUE_OR_DIE(initShaderPrograms(), "Cannot initialize shaders. Exit...");
    initTexture();
    initSceneMemory();
    initVertexArrayObjects();
    initSharedBlockUniform();
    initSceneMatrices();
}
//------------------------------------------------------------------------------------------
bool Renderer::initPhongShadingProgram()
{
    QOpenGLShaderProgram* program;
    GLint location;

    /////////////////////////////////////////////////////////////////
    glslPrograms[PhongShading] = new QOpenGLShaderProgram;
    program = glslPrograms[PhongShading];
    bool success;

    success = program->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                               vertexShaderSourceMap.value(PhongShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                               fragmentShaderSourceMap.value(PhongShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Geometry,
                                               ":/shaders/phong-shading.gs.glsl");
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    location = program->attributeLocation("v_coord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex coordinate.");
    attrVertex[PhongShading] = location;

    location = program->attributeLocation("v_normal");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex normal.");
    attrNormal[PhongShading] = location;

    location = program->attributeLocation("v_texCoord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex texCoord.");
    attrTexCoord[PhongShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Matrices");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMatrices[PhongShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Light");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniLight[PhongShading] = location;

    location = glGetUniformBlockIndex(program->programId(), "Material");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMaterial[PhongShading] = location;

    location = program->uniformLocation("cameraPosition");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform cameraPosition.");
    uniCameraPosition[PhongShading] = location;

    location = program->uniformLocation("ambientLight");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform ambientLight.");
    uniAmbientLight[PhongShading] = location;

    location = program->uniformLocation("objTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform objTex.");
    uniObjTexture[PhongShading] = location;

    location = program->uniformLocation("hasObjTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform hasObjTex.");
    uniHasObjTexture[PhongShading] = location;

    location = program->uniformLocation("normalTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform normalTex.");
    uniNormalTexture[PhongShading] = location;

    location = program->uniformLocation("hasNormalTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform hasNormalTex.");
    uniHasNormalTexture[PhongShading] = location;

    location = program->uniformLocation("depthTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform depthTex.");
    uniDepthTexture[PhongShading] = location;

    location = program->uniformLocation("hasDepthTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform hasDepthTex.");
    uniHasDepthTexture[PhongShading] = location;

    location = program->uniformLocation("needTangent");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform needTangent.");
    uniNeedTangent[PhongShading] = location;

    return true;
}


//------------------------------------------------------------------------------------------
bool Renderer::initShadowMapShadingProgram()
{
    GLint location;
    glslPrograms[ShadowMapShading] = new QOpenGLShaderProgram;
    QOpenGLShaderProgram* shadowMapProgram = glslPrograms[ShadowMapShading];
    bool success;

    success = shadowMapProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                        vertexShaderSourceMap.value(ShadowMapShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = shadowMapProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                        fragmentShaderSourceMap.value(ShadowMapShading));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = shadowMapProgram->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    location = shadowMapProgram->attributeLocation("v_coord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex coordinate.");
    attrVertex[ShadowMapShading] = location;

    location = shadowMapProgram->attributeLocation("v_texCoord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute texture coordinate.");
    attrTexCoord[ShadowMapShading] = location;

    location = glGetUniformBlockIndex(shadowMapProgram->programId(), "Matrices");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMatrices[ShadowMapShading] = location;

    location = shadowMapProgram->uniformLocation("objTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform objTex.");
    uniObjTexture[ShadowMapShading] = location;

    location = shadowMapProgram->uniformLocation("hasObjTex");
    TRUE_OR_DIE(location >= 0, "Cannot bind uniform hasObjTex.");
    uniHasObjTexture[ShadowMapShading] = location;

    return true;
}

//------------------------------------------------------------------------------------------
bool Renderer::initRenderLightProgram()
{
    GLint location;
    glslPrograms[ProgramRenderLight] = new QOpenGLShaderProgram;
    QOpenGLShaderProgram* program = glslPrograms[ProgramRenderLight];
    bool success;

    success = program->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                               vertexShaderSourceMap.value(ProgramRenderLight));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                               fragmentShaderSourceMap.value(ProgramRenderLight));
    TRUE_OR_DIE(success, "Cannot compile shader from file.");

    success = program->link();
    TRUE_OR_DIE(success, "Cannot link GLSL program.");

    location = program->attributeLocation("v_coord");
    TRUE_OR_DIE(location >= 0, "Cannot bind attribute vertex coordinate.");
    attrVertex[ProgramRenderLight] = location;

    location = glGetUniformBlockIndex(program->programId(), "Matrices");
    TRUE_OR_DIE(location >= 0, "Cannot bind block uniform.");
    uniMatrices[ProgramRenderLight] = location;

    return true;
}

//------------------------------------------------------------------------------------------
bool Renderer::initShaderPrograms()
{
    vertexShaderSourceMap.insert(PhongShading, ":/shaders/phong-shading.vs.glsl");
    vertexShaderSourceMap.insert(ShadowMapShading,
                                 ":/shaders/shadow-map.vs.glsl");
    vertexShaderSourceMap.insert(ProgramRenderLight,
                                 ":/shaders/light.vs.glsl");

    fragmentShaderSourceMap.insert(PhongShading, ":/shaders/phong-shading.fs.glsl");
    fragmentShaderSourceMap.insert(ShadowMapShading,
                                   ":/shaders/shadow-map.fs.glsl");
    fragmentShaderSourceMap.insert(ProgramRenderLight,
                                   ":/shaders/light.fs.glsl");


    return (initShadowMapShadingProgram() &&
            initRenderLightProgram() &&
            initPhongShadingProgram());
}

//------------------------------------------------------------------------------------------
bool Renderer::validateShaderPrograms(ShadingProgram _shadingMode)
{
    GLint status;
    GLint logLen;
    GLchar log[1024];

    glValidateProgram(glslPrograms[_shadingMode]->programId());
    glGetProgramiv(glslPrograms[_shadingMode]->programId(), GL_VALIDATE_STATUS, &status);

    glGetProgramiv(glslPrograms[_shadingMode]->programId(), GL_INFO_LOG_LENGTH, &logLen);

    if(logLen > 0)
    {
        glGetProgramInfoLog(glslPrograms[_shadingMode]->programId(), logLen, &logLen, log);

        if(QString(log).trimmed().length() != 0)
        {
            qDebug() << "ShadingMode: " << _shadingMode << ", log: " << log;
        }
    }

    return (status == GL_TRUE);
}

//------------------------------------------------------------------------------------------
void Renderer::initSharedBlockUniform()
{
    /////////////////////////////////////////////////////////////////
    // setup the light and material
    light.direction = DEFAULT_LIGHT_DIRECTION;
    light.intensity = 0.8f;

    roomMaterial.setDiffuse(QVector4D(1.0f, 0.85f, 0.45f, 1.0f));
    roomMaterial.setSpecular(QVector4D(0.5f, 0.5f, 0.5f, 1.0f));
    roomMaterial.shininess = 50.0f;

    cubeMaterial.setDiffuse(QVector4D(0.67f, 0.33f, 1.0f, 1.0f));
    cubeMaterial.setSpecular(QVector4D(0.5f, 0.5f, 0.5f, 1.0f));
    cubeMaterial.shininess = 50.0f;

    sphereMaterial.setDiffuse(QVector4D(0.0f, 1.0f, 0.2f, 1.0f));
    sphereMaterial.setSpecular(QVector4D(0.5f, 0.5f, 0.5f, 1.0f));
    sphereMaterial.shininess = 50.0f;

    meshObjectMaterial.setDiffuse(QVector4D(0.82f, 0.45f, 1.0f, 1.0f));
    meshObjectMaterial.setSpecular(QVector4D(0.5f, 0.5f, 0.5f, 1.0f));
    meshObjectMaterial.shininess = 50.0f;


    /////////////////////////////////////////////////////////////////
    // setup binding points for block uniform
    for(int i = 0; i < NUM_BINDING_POINTS; ++i)
    {
        UBOBindingIndex[i] = i + 1;
    }

    /////////////////////////////////////////////////////////////////
    // setup data for block uniform
    glGenBuffers(1, &UBOMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 4 * SIZE_OF_MAT4, NULL,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBOLight);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
    glBufferData(GL_UNIFORM_BUFFER, light.getStructSize(),
                 &light, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBORoomMaterial);
    glBindBuffer(GL_UNIFORM_BUFFER, UBORoomMaterial);
    glBufferData(GL_UNIFORM_BUFFER, roomMaterial.getStructSize(),
                 &roomMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBOCubeMaterial);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOCubeMaterial);
    glBufferData(GL_UNIFORM_BUFFER, cubeMaterial.getStructSize(),
                 &cubeMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBOSphereMaterial);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOSphereMaterial);
    glBufferData(GL_UNIFORM_BUFFER, sphereMaterial.getStructSize(),
                 &sphereMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &UBOMeshObjectMaterial);
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMeshObjectMaterial);
    glBufferData(GL_UNIFORM_BUFFER, meshObjectMaterial.getStructSize(),
                 &meshObjectMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

//------------------------------------------------------------------------------------------
void Renderer::initTexture()
{
    ////////////////////////////////////////////////////////////////////////////////
    // decal texture
    QString texFile = QString(":/textures/tiger.png");
    TRUE_OR_DIE(QFile::exists(texFile), "Cannot load texture from file.");
    decalTexture = new QOpenGLTexture(
        QImage(texFile).mirrored().convertToFormat(QImage::Format_RGBA8888));
    decalTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    decalTexture->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
//    decalTexture->setWrapMode(QOpenGLTexture::DirectionS,
//                              QOpenGLTexture::ClampToEdge);
//    decalTexture->setWrapMode(QOpenGLTexture::DirectionT,
//                              QOpenGLTexture::ClampToEdge);
    decalTexture->setWrapMode(QOpenGLTexture::DirectionS,
                              QOpenGLTexture::Repeat);
    decalTexture->setWrapMode(QOpenGLTexture::DirectionT,
                              QOpenGLTexture::Repeat);


    QString normalTexFile;
    QString colorTexFile;
    ////////////////////////////////////////////////////////////////////////////////
    // sphere texture

    normalTexFile = QString(":/textures/dog_bump_map.png");
    TRUE_OR_DIE(QFile::exists(normalTexFile), "Cannot load texture from file.");
    colorTexFile = QString(":/textures/dog_color_map.png");
    TRUE_OR_DIE(QFile::exists(colorTexFile), "Cannot load texture from file.");


    QImage bumpMapImage = QImage(normalTexFile).convertToFormat(
                              QImage::Format_Indexed8).mirrored();
    float scale = 24;
    int w = bumpMapImage.width();
    int h = bumpMapImage.height();


    QImage normalMapImage = QImage(w, h, QImage::Format_RGB888);

    for(int i = 1; i < w - 1; i++)
    {
        for(int j = 1; j < h - 1; j++)
        {
            float strength = scale / 16;

            int tl = bumpMapImage.pixelIndex(i - 1, j - 1);
            int l = bumpMapImage.pixelIndex(i - 1, j);
            int bl = bumpMapImage.pixelIndex(i - 1, j + 1);
            int b = bumpMapImage.pixelIndex(i, j + 1);
            int br = bumpMapImage.pixelIndex(i + 1, j + 1);
            int r = bumpMapImage.pixelIndex(i + 1, j);
            int tr = bumpMapImage.pixelIndex(i + 1, j - 1);
            int t = bumpMapImage.pixelIndex(i, j - 1);

            // Compute dx using Sobel:
            //           -1 0 1
            //           -2 0 2
            //           -1 0 1
            int dX = tr + 2 * r + br - tl - 2 * l - bl;

            // Compute dy using Sobel:
            //           -1 -2 -1
            //            0  0  0
            //            1  2  1
            int dY = bl + 2 * b + br - tl - 2 * t - tr;

            QVector3D n(dX / 255.0, dY / 255.0, 1.0f / strength);
            n.normalize();

            normalMapImage.setPixel(i, j, qRgb((int)((n.x() + 1.0) * 127.5),
                                               (int)((n.y() + 1.0) * 127.5),
                                               (int)((n.z() + 1.0) * 127.5)));
        }
    }


    // cheesy boundary cop-out
    for(int i = 0; i < w; i++)
    {
        normalMapImage.setPixel(i, 0, normalMapImage.pixel(i, 1));
        normalMapImage.setPixel(i, h - 1, normalMapImage.pixel(i, h - 2));
    }

    for(int j = 0; j < h; j++)
    {
        normalMapImage.setPixel(0, j, normalMapImage.pixel(1, j));
        normalMapImage.setPixel(w - 1, j, normalMapImage.pixel(w - 2, j));
    }

//    normalMapImage.save("/Users/nghia/tmp.png");


    normalMapsSphere = new QOpenGLTexture(normalMapImage);
    normalMapsSphere->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    normalMapsSphere->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
    normalMapsSphere->setWrapMode(QOpenGLTexture::Repeat);

    colorMapsSphere = new QOpenGLTexture(QImage(colorTexFile).mirrored());
    colorMapsSphere->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    colorMapsSphere->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
    colorMapsSphere->setWrapMode(QOpenGLTexture::Repeat);

    ////////////////////////////////////////////////////////////////////////////////
    // floor texture
    normalTexFile = QString(":/textures/floor_normal_map.png");
    TRUE_OR_DIE(QFile::exists(normalTexFile), "Cannot load texture from file.");
    normalMapsFloor[0] = new QOpenGLTexture(QImage(normalTexFile).mirrored());
    normalMapsFloor[0]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    normalMapsFloor[0]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
    normalMapsFloor[0]->setWrapMode(QOpenGLTexture::Repeat);

    for(int tex = 1; tex < NumFloorTextures; ++tex)
    {
        normalTexFile = QString(":/textures/floors/%1-NormalMap.png").
                        arg(strListFloorTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(normalTexFile), "Cannot load texture from file.");
        colorTexFile = QString(":/textures/floors/%1-ColorMap.png").
                       arg(strListFloorTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(colorTexFile), "Cannot load texture from file.");

        normalMapsFloor[tex] = new QOpenGLTexture(QImage(normalTexFile).mirrored());
        normalMapsFloor[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsFloor[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsFloor[tex]->setWrapMode(QOpenGLTexture::Repeat);

        colorMapsFloor[tex] = new QOpenGLTexture(QImage(colorTexFile).mirrored());
        colorMapsFloor[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsFloor[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsFloor[tex]->setWrapMode(QOpenGLTexture::Repeat);
    }


    ////////////////////////////////////////////////////////////////////////////////
    // wall texture
    for(int tex = 0; tex < NumWallTextures; ++tex)
    {
        normalTexFile = QString(":/textures/walls/%1-NormalMap.png").
                        arg(strListWallTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(normalTexFile), "Cannot load texture from file.");
        colorTexFile = QString(":/textures/walls/%1-ColorMap.png").
                       arg(strListWallTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(colorTexFile), "Cannot load texture from file.");

        normalMapsWall[tex] = new QOpenGLTexture(QImage(normalTexFile).mirrored());
        normalMapsWall[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsWall[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsWall[tex]->setWrapMode(QOpenGLTexture::Repeat);

        colorMapsWall[tex] = new QOpenGLTexture(QImage(colorTexFile).mirrored());
        colorMapsWall[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsWall[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsWall[tex]->setWrapMode(QOpenGLTexture::Repeat);
    }


    ////////////////////////////////////////////////////////////////////////////////
    // mesh object texture
    for(int tex = 0; tex < NumMetalTextures; ++tex)
    {
        normalTexFile = QString(":/textures/metals/%1-NormalMap.png").
                        arg(strListMeshObjectTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(normalTexFile), "Cannot load texture from file.");
        colorTexFile = QString(":/textures/metals/%1-ColorMap.png").
                       arg(strListMeshObjectTexture->at(tex));
        TRUE_OR_DIE(QFile::exists(colorTexFile), "Cannot load texture from file.");

        normalMapsMeshObject[tex] = new QOpenGLTexture(QImage(normalTexFile).mirrored());
        normalMapsMeshObject[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsMeshObject[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        normalMapsMeshObject[tex]->setWrapMode(QOpenGLTexture::Repeat);

        colorMapsMeshObject[tex] = new QOpenGLTexture(QImage(colorTexFile).mirrored());
        colorMapsMeshObject[tex]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsMeshObject[tex]->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
        colorMapsMeshObject[tex]->setWrapMode(QOpenGLTexture::Repeat);
    }
}

//------------------------------------------------------------------------------------------
void Renderer::initSceneMemory()
{
    initLightObjectMemory();
    initRoomMemory();
    initCubeMemory();
    initSphereMemory();
    initMeshObjectMemory();
}

//------------------------------------------------------------------------------------------
void Renderer::initRoomMemory()
{
    if(!cubeObject)
    {
        cubeObject = new UnitCube;
    }

    if(vboRoom.isCreated())
    {
        vboRoom.destroy();
    }

    if(iboRoom.isCreated())
    {
        iboRoom.destroy();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // init memory for cube
    vboRoom.create();
    vboRoom.bind();
    vboRoom.allocate(2 * cubeObject->getVertexOffset() + cubeObject->getTexCoordOffset());
    vboRoom.write(0, cubeObject->getVertices(), cubeObject->getVertexOffset());
    vboRoom.write(cubeObject->getVertexOffset(), cubeObject->getNegativeNormals(),
                  cubeObject->getVertexOffset());
    vboRoom.write(2 * cubeObject->getVertexOffset(), cubeObject->getTexureCoordinates(1.0f),
                  cubeObject->getTexCoordOffset());
    vboRoom.release();
    // indices
    iboRoom.create();
    iboRoom.bind();
    iboRoom.allocate(cubeObject->getIndices(), cubeObject->getIndexOffset());
    iboRoom.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initCubeMemory()
{
    if(!cubeObject)
    {
        cubeObject = new UnitCube;
    }

    if(vboCube.isCreated())
    {
        vboCube.destroy();
    }

    if(iboCube.isCreated())
    {
        iboCube.destroy();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // init memory for cube
    vboCube.create();
    vboCube.bind();
    vboCube.allocate(2 * cubeObject->getVertexOffset() + cubeObject->getTexCoordOffset());
    vboCube.write(0, cubeObject->getVertices(), cubeObject->getVertexOffset());
    vboCube.write(cubeObject->getVertexOffset(), cubeObject->getNormals(),
                  cubeObject->getVertexOffset());
    vboCube.write(2 * cubeObject->getVertexOffset(), cubeObject->getTexureCoordinates(1.0f),
                  cubeObject->getTexCoordOffset());
    vboCube.release();
    // indices
    iboCube.create();
    iboCube.bind();
    iboCube.allocate(cubeObject->getIndices(), cubeObject->getIndexOffset());
    iboCube.release();
}


//------------------------------------------------------------------------------------------
void Renderer::initSphereMemory()
{
    if(!sphereObject)
    {
        sphereObject = new UnitSphere;
        sphereObject->generateSphere(25, 50);
    }

    if(vboSphere.isCreated())
    {
        vboSphere.destroy();
    }

    if(iboSphere.isCreated())
    {
        iboSphere.destroy();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // init memory for sphere
    vboSphere.create();
    vboSphere.bind();
    vboSphere.allocate(2 * sphereObject->getVertexOffset() +
                       sphereObject->getTexCoordOffset());
    vboSphere.write(0, sphereObject->getVertices(), sphereObject->getVertexOffset());
    vboSphere.write(sphereObject->getVertexOffset(), sphereObject->getNormals(),
                    sphereObject->getVertexOffset());
    vboSphere.write(2 * sphereObject->getVertexOffset(), sphereObject->getTexureCoordinates(),
                    sphereObject->getTexCoordOffset());
    vboSphere.release();
    // indices
    iboSphere.create();
    iboSphere.bind();
    iboSphere.allocate(sphereObject->getIndices(), sphereObject->getIndexOffset());
    iboSphere.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initMeshObjectMemory()
{
    if(!objLoader)
    {
        objLoader = new OBJLoader;
    }

    bool result = false;

    switch (currentMeshObject)
    {
    case TEAPOT_OBJ:
        result = objLoader->loadObjFile(":/obj/teapot.obj");
        break;

    case BUNNY_OBJ:
        result = objLoader->loadObjFile(":/obj/bunny.obj");
        break;

    default:
        break;
    }

    if(!result)
    {
        QMessageBox::critical(NULL, "Error", "Could not load OBJ file!");
        return;
    }

    if(vboMeshObject.isCreated())
    {
        vboMeshObject.destroy();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // init memory for sphere
    vboMeshObject.create();
    vboMeshObject.bind();
    vboMeshObject.allocate(2 * objLoader->getVertexOffset() +
                           objLoader->getTexCoordOffset());
    vboMeshObject.write(0, objLoader->getVertices(), objLoader->getVertexOffset());
    vboMeshObject.write(objLoader->getVertexOffset(), objLoader->getNormals(),
                        objLoader->getVertexOffset());
    vboMeshObject.write(2 * objLoader->getVertexOffset(), objLoader->getTexureCoordinates(),
                        objLoader->getTexCoordOffset());
    vboMeshObject.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initLightObjectMemory()
{
    if(vboLight.isCreated())
    {
        vboLight.destroy();
    }


    QVector3D lightPoint1(-DEFAULT_LIGHT_DIRECTION * 1000.0);
    QVector3D lightPoint2(DEFAULT_LIGHT_DIRECTION * 1000.0);

    ////////////////////////////////////////////////////////////////////////////////
    // init memory for directional light rendering
    vboLight.create();
    vboLight.bind();
    vboLight.allocate(6 * sizeof(GLfloat));
    vboLight.write(0, &lightPoint1, sizeof(GLfloat) * 3);
    vboLight.write(sizeof(GLfloat) * 3, &lightPoint2, sizeof(GLfloat) * 3);
    vboLight.release();
}

//------------------------------------------------------------------------------------------
// record the buffer state by vertex array object
//------------------------------------------------------------------------------------------
void Renderer::initVertexArrayObjects()
{
    initLightVAO();

    initRoomVAO(PhongShading);
    initRoomVAO(ShadowMapShading);

    initCubeVAO(PhongShading);
    initCubeVAO(ShadowMapShading);

    initSphereVAO(PhongShading);
    initSphereVAO(ShadowMapShading);

    initMeshObjectVAO(PhongShading);
    initMeshObjectVAO(ShadowMapShading);
}

//------------------------------------------------------------------------------------------
void Renderer::initRoomVAO(ShadingProgram _shadingMode)
{
    if(vaoRoom[_shadingMode].isCreated())
    {
        vaoRoom[_shadingMode].destroy();
    }

    QOpenGLShaderProgram* program = glslPrograms[_shadingMode];

    vaoRoom[_shadingMode].create();
    vaoRoom[_shadingMode].bind();

    vboRoom.bind();
    program->enableAttributeArray(attrVertex[_shadingMode]);
    program->setAttributeBuffer(attrVertex[_shadingMode], GL_FLOAT, 0, 3);

    program->enableAttributeArray(attrNormal[_shadingMode]);
    program->setAttributeBuffer(attrNormal[_shadingMode], GL_FLOAT,
                                cubeObject->getVertexOffset(), 3);

    program->enableAttributeArray(attrTexCoord[_shadingMode]);
    program->setAttributeBuffer(attrTexCoord[_shadingMode], GL_FLOAT,
                                2 * cubeObject->getVertexOffset(), 2);

    iboRoom.bind();

    // release vao before vbo and ibo
    vaoRoom[_shadingMode].release();
    vboRoom.release();
    iboRoom.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initCubeVAO(ShadingProgram _shadingMode)
{
    if(vaoCube[_shadingMode].isCreated())
    {
        vaoCube[_shadingMode].destroy();
    }

    QOpenGLShaderProgram* program = glslPrograms[_shadingMode];

    vaoCube[_shadingMode].create();
    vaoCube[_shadingMode].bind();

    vboCube.bind();
    program->enableAttributeArray(attrVertex[_shadingMode]);
    program->setAttributeBuffer(attrVertex[_shadingMode], GL_FLOAT, 0, 3);

    if(_shadingMode == PhongShading)
    {
        program->enableAttributeArray(attrNormal[_shadingMode]);
        program->setAttributeBuffer(attrNormal[_shadingMode], GL_FLOAT,
                                    cubeObject->getVertexOffset(), 3);

        program->enableAttributeArray(attrTexCoord[_shadingMode]);
        program->setAttributeBuffer(attrTexCoord[_shadingMode], GL_FLOAT,
                                    2 * cubeObject->getVertexOffset(), 2);
    }

    iboCube.bind();

    // release vao before vbo and ibo
    vaoCube[_shadingMode].release();
    vboCube.release();
    iboCube.release();

}


//------------------------------------------------------------------------------------------
void Renderer::initSphereVAO(ShadingProgram _shadingMode)
{
    if(vaoSphere[_shadingMode].isCreated())
    {
        vaoSphere[_shadingMode].destroy();
    }

    QOpenGLShaderProgram* program = glslPrograms[_shadingMode];

    vaoSphere[_shadingMode].create();
    vaoSphere[_shadingMode].bind();

    vboSphere.bind();
    program->enableAttributeArray(attrVertex[_shadingMode]);
    program->setAttributeBuffer(attrVertex[_shadingMode], GL_FLOAT, 0, 3);

    if(_shadingMode == PhongShading)
    {
        program->enableAttributeArray(attrNormal[_shadingMode]);
        program->setAttributeBuffer(attrNormal[_shadingMode], GL_FLOAT,
                                    sphereObject->getVertexOffset(), 3);

        program->enableAttributeArray(attrTexCoord[_shadingMode]);
        program->setAttributeBuffer(attrTexCoord[_shadingMode], GL_FLOAT,
                                    2 * sphereObject->getVertexOffset(), 2);
    }

    iboSphere.bind();

    // release vao before vbo and ibo
    vaoSphere[_shadingMode].release();
    vboSphere.release();
    iboSphere.release();

}

//------------------------------------------------------------------------------------------
void Renderer::initMeshObjectVAO(ShadingProgram _shadingMode)
{
    if(vaoMeshObject[_shadingMode].isCreated())
    {
        vaoMeshObject[_shadingMode].destroy();
    }

    QOpenGLShaderProgram* program = glslPrograms[_shadingMode];

    vaoMeshObject[_shadingMode].create();
    vaoMeshObject[_shadingMode].bind();

    vboMeshObject.bind();
    program->enableAttributeArray(attrVertex[_shadingMode]);
    program->setAttributeBuffer(attrVertex[_shadingMode], GL_FLOAT, 0, 3);

    if(_shadingMode == PhongShading)
    {
        program->enableAttributeArray(attrNormal[_shadingMode]);
        program->setAttributeBuffer(attrNormal[_shadingMode], GL_FLOAT,
                                    objLoader->getVertexOffset(), 3);

        program->enableAttributeArray(attrTexCoord[_shadingMode]);
        program->setAttributeBuffer(attrTexCoord[_shadingMode], GL_FLOAT,
                                    2 * objLoader->getVertexOffset(), 2);
    }


    // release vao before vbo and ibo
    vaoMeshObject[_shadingMode].release();
    vboMeshObject.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initLightVAO()
{
    if(vaoLight.isCreated())
    {
        vaoLight.destroy();
    }

    QOpenGLShaderProgram* program = glslPrograms[ProgramRenderLight];

    vaoLight.create();
    vaoLight.bind();

    vboLight.bind();
    program->enableAttributeArray(attrVertex[ProgramRenderLight]);
    program->setAttributeBuffer(attrVertex[ProgramRenderLight], GL_FLOAT, 0, 3);

    vaoLight.release();
    vboLight.release();
}

//------------------------------------------------------------------------------------------
void Renderer::initSceneMatrices()
{
    /////////////////////////////////////////////////////////////////
    // room
    setRoomSize(3);

    /////////////////////////////////////////////////////////////////
    // cube
    cubeModelMatrix.setToIdentity();
    cubeModelMatrix.scale(2.0f);
    cubeModelMatrix.translate(DEFAULT_CUBE_POSITION);
    cubeNormalMatrix = QMatrix4x4(cubeModelMatrix.normalMatrix());

    /////////////////////////////////////////////////////////////////
    // reflective sphere
    sphereModelMatrix.setToIdentity();
    sphereModelMatrix.translate(DEFAULT_SPHERE_POSITION);
    sphereModelMatrix.scale(2.0f);
    sphereNormalMatrix = QMatrix4x4(sphereModelMatrix.normalMatrix());

    /////////////////////////////////////////////////////////////////
    // mesh object
    TRUE_OR_DIE(objLoader, "OBJLoader must be initialized first");
    meshObjectModelMatrix.setToIdentity();
    meshObjectModelMatrix.translate(DEFAULT_MESH_OBJECT_POSITION);
    meshObjectModelMatrix.scale(3.0f);

    if(currentMeshObject != TEAPOT_OBJ)
        meshObjectModelMatrix.translate(QVector3D(0, -2.0f * objLoader->getLowestYCoordinate(),
                                                  0));

    meshObjectModelMatrix.scale(2.0f / objLoader->getScalingFactor());

    if(currentMeshObject == TEAPOT_OBJ)
    {
        meshObjectModelMatrix.rotate(-90, 1, 0, 0);
    }

    meshObjectNormalMatrix = QMatrix4x4(meshObjectModelMatrix.normalMatrix());

}

//------------------------------------------------------------------------------------------
void Renderer::initDepthBufferObject()
{
    if(depthTexture)
    {
        depthTexture->destroy();
        delete depthTexture;
    }

    depthTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    depthTexture->create();
    depthTexture->setSize(DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
    depthTexture->setFormat(QOpenGLTexture::D32);
    depthTexture->allocateStorage();
    depthTexture->setMinificationFilter(QOpenGLTexture::Linear);
    depthTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    depthTexture->setWrapMode(QOpenGLTexture::DirectionS,
                              QOpenGLTexture::ClampToEdge);
    depthTexture->setWrapMode(QOpenGLTexture::DirectionT,
                              QOpenGLTexture::ClampToEdge);

    depthTexture->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // frame buffer
    FBODepthMap = new QOpenGLFramebufferObject(DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
    FBODepthMap->bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         depthTexture->textureId(), 0);
    TRUE_OR_DIE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                "Framebuffer is imcomplete!");
    FBODepthMap->release();

    // shadow matrix
    lightProjectionMatrix.setToIdentity();
    lightProjectionMatrix.ortho(-20, 20, -20, 20, -1000, 1000);
    lightViewMatrix.setToIdentity();
    lightViewMatrix.lookAt(-50 * QVector3D(light.direction), QVector3D(0, 0, 0), QVector3D(0,
                                                                                           0,
                                                                                           -1));
    shadowMatrix = lightProjectionMatrix * lightViewMatrix;

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * SIZE_OF_MAT4, SIZE_OF_MAT4,
                    shadowMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    initializedDepthBuffer = true;
}

//------------------------------------------------------------------------------------------
void Renderer::setRoomSize(int _roomSize)
{
    if(isValid())
    {
        makeCurrent();
    }

    roomSize = (float)_roomSize;
    roomModelMatrix.setToIdentity();
    roomModelMatrix.scale(roomSize * 4);
    roomModelMatrix.translate(0.0, 1.0, 0.0);
    roomNormalMatrix = QMatrix4x4(roomModelMatrix.normalMatrix());

    vboRoom.bind();
    vboRoom.write(2 * cubeObject->getVertexOffset(),
                  cubeObject->getTexureCoordinates(roomSize),
                  cubeObject->getTexCoordOffset());
    vboRoom.release();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::setAmbientLightIntensity(int _ambientLight)
{
    ambientLight = (float) _ambientLight / 100.0f;
}

//------------------------------------------------------------------------------------------
void Renderer::setLightIntensity(int _intensity)
{
    if(!isValid())
    {
        return;
    }

    makeCurrent();
    light.intensity = (GLfloat)_intensity / 100.0f;
    glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
    glBufferData(GL_UNIFORM_BUFFER, light.getStructSize(),
                 &light, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::setObjectsSpecularReflection(int _intensity)
{
    if(!isValid())
    {
        return;
    }

    makeCurrent();
    float specular = (float) _intensity / 100.0f;
    roomMaterial.setSpecular(QVector4D(specular, specular, specular, 1.0f));
    cubeMaterial.setSpecular(QVector4D(specular, specular, specular, 1.0f));
    sphereMaterial.setSpecular(QVector4D(specular, specular, specular, 1.0f));
    meshObjectMaterial.setSpecular(QVector4D(specular, specular, specular, 1.0f));

    glBindBuffer(GL_UNIFORM_BUFFER, UBORoomMaterial);
    glBufferData(GL_UNIFORM_BUFFER, roomMaterial.getStructSize(),
                 &roomMaterial, GL_STREAM_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, UBOCubeMaterial);
    glBufferData(GL_UNIFORM_BUFFER, cubeMaterial.getStructSize(),
                 &cubeMaterial, GL_STREAM_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, UBOSphereMaterial);
    glBufferData(GL_UNIFORM_BUFFER, sphereMaterial.getStructSize(),
                 &sphereMaterial, GL_STREAM_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMeshObjectMaterial);
    glBufferData(GL_UNIFORM_BUFFER, meshObjectMaterial.getStructSize(),
                 &meshObjectMaterial, GL_STREAM_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::resetObjectPositions()
{
    makeCurrent();
    initSceneMatrices();
    doneCurrent();
}

//------------------------------------------------------------------------------------------
void Renderer::resetLightDirection()
{
    makeCurrent();
    light.direction = DEFAULT_LIGHT_DIRECTION;
    glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
    glBufferData(GL_UNIFORM_BUFFER, light.getStructSize(),
                 &light, GL_STREAM_DRAW);

    lightViewMatrix.setToIdentity();
    lightViewMatrix.lookAt(-50 * QVector3D(light.direction), QVector3D(0, 0, 0), QVector3D(0,
                                                                                           0,
                                                                                           -1));
    shadowMatrix = lightProjectionMatrix * lightViewMatrix;

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * SIZE_OF_MAT4, SIZE_OF_MAT4,
                    shadowMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
}

//------------------------------------------------------------------------------------------
void Renderer::setMeshObject(int _objectIndex)
{
    if(!isValid())
    {
        return;
    }

    if(_objectIndex < 0 || _objectIndex >= NUM_MESH_OBJECT)
    {
        return;
    }

    currentMeshObject = static_cast<MeshObject>(_objectIndex);
    makeCurrent();
    initMeshObjectMemory();
    initMeshObjectVAO(PhongShading);
    initMeshObjectVAO(ShadowMapShading);

    /////////////////////////////////////////////////////////////////
    // mesh object
    TRUE_OR_DIE(objLoader, "OBJLoader must be initialized first");
    meshObjectModelMatrix.setToIdentity();
    meshObjectModelMatrix.translate(DEFAULT_MESH_OBJECT_POSITION);
    meshObjectModelMatrix.scale(3.0f);

    if(currentMeshObject != TEAPOT_OBJ)
        meshObjectModelMatrix.translate(QVector3D(0, -2.0f * objLoader->getLowestYCoordinate(),
                                                  0));

    meshObjectModelMatrix.scale(2.0f / objLoader->getScalingFactor());

    if(currentMeshObject == TEAPOT_OBJ)
    {
        meshObjectModelMatrix.rotate(-90, 1, 0, 0);
    }

    meshObjectNormalMatrix = QMatrix4x4(meshObjectModelMatrix.normalMatrix());

    doneCurrent();
}

//------------------------------------------------------------------------------------------
void Renderer::setTextureFilteringMode(QOpenGLTexture::Filter
                                       _textureFiltering)
{
    for(int i = 0; i < NumFloorTextures; ++i)
    {
        normalMapsFloor[i]->setMinMagFilters(_textureFiltering, _textureFiltering);
    }
}

//------------------------------------------------------------------------------------------
QStringList* Renderer::getStrListFloorTexture()
{
    return strListFloorTexture;
}

//------------------------------------------------------------------------------------------
QStringList* Renderer::getStrListWallTexture()
{
    return strListWallTexture;
}

//------------------------------------------------------------------------------------------
QStringList* Renderer::getStrListMeshObjectTexture()
{
    return strListMeshObjectTexture;
}

//------------------------------------------------------------------------------------------
void Renderer::setCubeColor(float _r, float _g, float _b)
{
    if(!isValid())
    {
        return;
    }

    cubeMaterial.setDiffuse(QVector4D(_r, _g, _b, 1.0f));
    makeCurrent();
    glBindBuffer(GL_UNIFORM_BUFFER, UBOCubeMaterial);
    glBufferData(GL_UNIFORM_BUFFER, cubeMaterial.getStructSize(),
                 &cubeMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
}

//------------------------------------------------------------------------------------------
void Renderer::setMeshObjectColor(float _r, float _g, float _b)
{
    if(!isValid())
    {
        return;
    }

    meshObjectMaterial.setDiffuse(QVector4D(_r, _g, _b, 1.0f));
    makeCurrent();
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMeshObjectMaterial);
    glBufferData(GL_UNIFORM_BUFFER, meshObjectMaterial.getStructSize(),
                 &meshObjectMaterial, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    doneCurrent();
}

//------------------------------------------------------------------------------------------
void Renderer::setFloorTexture(int _texture)
{
    currentFloorTexture = _texture;
}

//------------------------------------------------------------------------------------------
void Renderer::setWallTexture(int _texture)
{
    currentWallTexture = _texture;
}

//------------------------------------------------------------------------------------------
void Renderer::setMeshObjectTexture(int _texture)
{
    currentMeshObjectTexture = _texture;
}

//------------------------------------------------------------------------------------------
void Renderer::updateCamera()
{
    zoomCamera();

    /////////////////////////////////////////////////////////////////
    // flush camera data to uniform buffer
    viewMatrix.setToIdentity();
    viewMatrix.lookAt(cameraPosition, cameraFocus, cameraUpDirection);

    viewProjectionMatrix = projectionMatrix * viewMatrix;

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * SIZE_OF_MAT4, SIZE_OF_MAT4,
                    viewProjectionMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

//------------------------------------------------------------------------------------------
QSize Renderer::sizeHint() const
{
    return QSize(1600, 1200);
}

//------------------------------------------------------------------------------------------
QSize Renderer::minimumSizeHint() const
{
    return QSize(50, 50);
}

//------------------------------------------------------------------------------------------
void Renderer::initializeGL()
{
    initializeOpenGLFunctions();
    checkOpenGLVersion();

    if(!initializedScene)
    {
        initScene();
        initializedScene = true;
    }
}

//------------------------------------------------------------------------------------------
void Renderer::resizeGL(int w, int h)
{
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(45, (float)w / (float)h, 0.1f, 1000.0f);
}

//------------------------------------------------------------------------------------------
void Renderer::paintGL()
{
    if(!initializedScene)
    {
        return;
    }

    switch(currentMouseTransTarget)
    {
    case TRANSFORM_CAMERA:
    {
        translateCamera();
        rotateCamera();
    }
    break;

    case TRANSFORM_LIGHT:
    {
        rotateLight();
    }
    break;

    case TRANSFORM_CUBE_SPHERE:
    {
        translateCubeSphere();
        rotateCubeSphere();
    }
    break;

    case TRANSFORM_MESH_OBJECT:
    {
        translateMeshObject();
        rotateMeshObject();
    }
    break;
    }


    updateCamera();


    // render scene
    renderScene();

}

//-----------------------------------------------------------------------------------------
void Renderer::mousePressEvent(QMouseEvent* _event)
{
    lastMousePos = QVector2D(_event->localPos());

    if(_event->button() == Qt::RightButton)
    {
        mouseButtonPressed = RIGHT_BUTTON;
    }
    else
    {
        mouseButtonPressed = LEFT_BUTTON;

//        if(currentMouseTransTarget == TRANSFORM_LIGHT)
//        {
//            mouseButtonPressed = RIGHT_BUTTON;
//        }
    }
}

//-----------------------------------------------------------------------------------------
void Renderer::mouseMoveEvent(QMouseEvent* _event)
{
    QVector2D mouseMoved = QVector2D(_event->localPos()) - lastMousePos;

    switch(specialKeyPressed)
    {
    case Renderer::NO_KEY:
    {

        if(mouseButtonPressed == RIGHT_BUTTON)
        {
            translation.setX(translation.x() + mouseMoved.x() / 50.0f);
            translation.setY(translation.y() - mouseMoved.y() / 50.0f);
        }
        else
        {
            rotation.setX(rotation.x() - mouseMoved.x() / 5.0f);
            rotation.setY(rotation.y() - mouseMoved.y() / 5.0f);

            QPointF center = QPointF(0.5 * width(), 0.5 * height());
            QPointF escentricity = _event->localPos() - center;
            escentricity.setX(escentricity.x() / center.x());
            escentricity.setY(escentricity.y() / center.y());
            rotation.setZ(rotation.z() - (mouseMoved.x()*escentricity.y() - mouseMoved.y() *
                                          escentricity.x()) / 5.0f);
        }

    }
    break;

    case Renderer::SHIFT_KEY:
    {
        if(mouseButtonPressed == RIGHT_BUTTON)
        {
            QVector2D dir = mouseMoved.normalized();
            zooming += mouseMoved.length() * dir.x() / 500.0f;
        }
        else
        {
            rotation.setX(rotation.x() + mouseMoved.x() / 5.0f);
            rotation.setZ(rotation.z() + mouseMoved.y() / 5.0f);
        }
    }
    break;

    case Renderer::CTRL_KEY:
        break;

    }

    lastMousePos = QVector2D(_event->localPos());
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::mouseReleaseEvent(QMouseEvent* _event)
{
    mouseButtonPressed = NO_BUTTON;
}

//------------------------------------------------------------------------------------------
void Renderer::wheelEvent(QWheelEvent* _event)
{
    if(!_event->angleDelta().isNull())
    {
        zooming +=  (_event->angleDelta().x() + _event->angleDelta().y()) / 500.0f;
    }

    update();
}

//------------------------------------------------------------------------------------------
void Renderer::setShadingMode(ShadingProgram _shadingMode)
{
    currentShadingMode = _shadingMode;
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::resetCameraPosition()
{
    cameraPosition = DEFAULT_CAMERA_POSITION;
    cameraFocus = DEFAULT_CAMERA_FOCUS;
    cameraUpDirection = QVector3D(0.0f, 1.0f, 0.0f);

    update();
}

//------------------------------------------------------------------------------------------
void Renderer::enableDepthTest(bool _status)
{
    if(!isValid())
    {
        return;
    }

    makeCurrent();

    if(_status)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    doneCurrent();
    update();
}

//------------------------------------------------------------------------------------------
void Renderer::enableRenderLight(bool _state)
{
    enabledRenderLight = _state;
}

//------------------------------------------------------------------------------------------
void Renderer::setMouseTransformationTarget(MouseTransformationTarget _mouseTarget)
{
    currentMouseTransTarget = _mouseTarget;
}

//------------------------------------------------------------------------------------------
void Renderer::setShadowMethod(ShadowModes _shadowMode)
{
    currentShadowMode = _shadowMode;
}

//------------------------------------------------------------------------------------------
void Renderer::keyPressEvent(QKeyEvent* _event)
{
    switch(_event->key())
    {
    case Qt::Key_Shift:
        specialKeyPressed = Renderer::SHIFT_KEY;
        break;

    case Qt::Key_Plus:
        zooming -= 0.1f;
        break;

    case Qt::Key_Minus:
        zooming += 0.1f;
        break;

    case Qt::Key_Up:
        translation += QVector3D(0.0f, 0.5f, 0.0f);
        break;

    case Qt::Key_Down:
        translation -= QVector3D(0.0f, 0.5f, 0.0f);
        break;

    case Qt::Key_Left:
        translation -= QVector3D(0.5f, 0.0f, 0.0f);
        break;

    case Qt::Key_Right:
        translation += QVector3D(0.5f, 0.0f, 0.0f);
        break;

    default:
        QOpenGLWidget::keyPressEvent(_event);
    }
}

//------------------------------------------------------------------------------------------
void Renderer::keyReleaseEvent(QKeyEvent* _event)
{
    specialKeyPressed = Renderer::NO_KEY;
}

//------------------------------------------------------------------------------------------
void Renderer::translateCamera()
{
    translation *= MOVING_INERTIA;

    if(translation.lengthSquared() < 1e-4)
    {
        return;
    }


    QVector3D eyeVector = cameraFocus - cameraPosition;
    float scale = sqrt(eyeVector.length()) * 0.01f;

    QVector3D u = cameraUpDirection;

    QVector3D v = QVector3D::crossProduct(eyeVector, u);
    u = QVector3D::crossProduct(v, eyeVector);
    u.normalize();
    v.normalize();

    cameraPosition -= scale * (translation.x() * v + translation.y() * u);
    cameraFocus -= scale * (translation.x() * v + translation.y() * u);
}

//------------------------------------------------------------------------------------------
void Renderer::rotateCamera()
{
    rotation *= MOVING_INERTIA;

    if(rotation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D nEyeVector = cameraPosition - cameraFocus ;
    QVector3D u = cameraUpDirection;
    QVector3D v = QVector3D::crossProduct(-nEyeVector, u);

    u = QVector3D::crossProduct(v, -nEyeVector);
    u.normalize();
    v.normalize();

    float scale = sqrt(nEyeVector.length()) * 0.02f;
    QQuaternion qRotation = QQuaternion::fromAxisAndAngle(v, rotation.y() * scale) *
                            QQuaternion::fromAxisAndAngle(u, rotation.x() * scale) *
                            QQuaternion::fromAxisAndAngle(nEyeVector, rotation.z() * scale);
    nEyeVector = qRotation.rotatedVector(nEyeVector);

    QQuaternion qCamRotation = QQuaternion::fromAxisAndAngle(v, rotation.y() * scale) *
                               QQuaternion::fromAxisAndAngle(nEyeVector, rotation.z() * scale);

    cameraPosition = cameraFocus + nEyeVector;
    cameraUpDirection = qCamRotation.rotatedVector(cameraUpDirection);
}

//------------------------------------------------------------------------------------------
void Renderer::zoomCamera()
{
    zooming *= MOVING_INERTIA;

    if(fabs(zooming) < 1e-4)
    {
        return;
    }

    QVector3D nEyeVector = cameraPosition - cameraFocus ;
    float len = nEyeVector.length();
    nEyeVector.normalize();

    len += sqrt(len) * zooming * 0.3f;

    if(len < 0.5f)
    {
        len = 0.5f;
    }

    cameraPosition = len * nEyeVector + cameraFocus;

}

//------------------------------------------------------------------------------------------
void Renderer::translateCubeSphere()
{
    translation *= MOVING_INERTIA;

    if(translation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D eyeVector = cameraFocus - cameraPosition;
    float scale = sqrt(eyeVector.length()) * 0.05f;

    QVector3D u = cameraUpDirection;
    QVector3D v = QVector3D::crossProduct(eyeVector, u);
    u = QVector3D::crossProduct(v, eyeVector);
    u.normalize();
    v.normalize();

    QVector3D objectTrans = scale * (translation.x() * v + translation.y() * u);
    QMatrix4x4 translationMatrix;
    translationMatrix.setToIdentity();
    translationMatrix.translate(objectTrans);

    cubeModelMatrix = translationMatrix * cubeModelMatrix;
    sphereModelMatrix = translationMatrix * sphereModelMatrix;
}

//------------------------------------------------------------------------------------------
void Renderer::rotateCubeSphere()
{
    rotation *= MOVING_INERTIA;

    if(rotation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D currentPos(0.0f, 0.0f, 0.0f);
    currentPos = cubeModelMatrix * currentPos;

    float scale = -0.2f;
    QQuaternion qRotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f),
                                                          rotation.x() * scale) *
                            QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), rotation.y() * scale);
    //*
    //                      QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), rotation.z()*scale);

    QMatrix4x4 rotationMatrix;
    rotationMatrix.setToIdentity();
    rotationMatrix.rotate(qRotation);

    QMatrix4x4 invTranslationMatrix, translationMatrix;
    invTranslationMatrix.setToIdentity();
    invTranslationMatrix.translate(-1.0f * currentPos);
    translationMatrix.setToIdentity();
    translationMatrix.translate(currentPos);

    cubeModelMatrix = translationMatrix * rotationMatrix * invTranslationMatrix *
                      cubeModelMatrix;
    sphereModelMatrix = translationMatrix * rotationMatrix *
                        invTranslationMatrix *
                        sphereModelMatrix;

    cubeNormalMatrix = QMatrix4x4(cubeModelMatrix.normalMatrix());
    sphereNormalMatrix = QMatrix4x4(sphereModelMatrix.normalMatrix());
}


//------------------------------------------------------------------------------------------
void Renderer::translateMeshObject()
{
    translation *= MOVING_INERTIA;

    if(translation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D eyeVector = cameraFocus - cameraPosition;
    float scale = sqrt(eyeVector.length()) * 0.05f;

    QVector3D u = cameraUpDirection;
    QVector3D v = QVector3D::crossProduct(eyeVector, u);
    u = QVector3D::crossProduct(v, eyeVector);
    u.normalize();
    v.normalize();

    QVector3D objectTrans = scale * (translation.x() * v + translation.y() * u);
    QMatrix4x4 translationMatrix;
    translationMatrix.setToIdentity();
    translationMatrix.translate(objectTrans);

    meshObjectModelMatrix = translationMatrix * meshObjectModelMatrix;
}

//------------------------------------------------------------------------------------------
void Renderer::rotateMeshObject()
{
    rotation *= MOVING_INERTIA;

    if(rotation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D currentPos(0.0f, 0.0f, 0.0f);
    currentPos =  meshObjectModelMatrix * currentPos;

    float scale = -0.2f;
    QQuaternion qRotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f),
                                                          rotation.x() * scale) *
                            QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), rotation.y() * scale);
    //*
    //                      QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), rotation.z()*scale);

    QMatrix4x4 rotationMatrix;
    rotationMatrix.setToIdentity();
    rotationMatrix.rotate(qRotation);

    QMatrix4x4 invTranslationMatrix, translationMatrix;
    invTranslationMatrix.setToIdentity();
    invTranslationMatrix.translate(-1.0f * currentPos);
    translationMatrix.setToIdentity();
    translationMatrix.translate(currentPos);

    meshObjectModelMatrix = translationMatrix * rotationMatrix * invTranslationMatrix *
                            meshObjectModelMatrix;

    meshObjectNormalMatrix = QMatrix4x4( meshObjectModelMatrix.normalMatrix());
}

//------------------------------------------------------------------------------------------
void Renderer::rotateLight()
{
    rotation *= MOVING_INERTIA;

    if(rotation.lengthSquared() < 1e-4)
    {
        return;
    }

    QVector3D eyeVector = cameraFocus - cameraPosition;
    float scale = -0.1f;

    QVector3D u = cameraUpDirection;
    QVector3D v = QVector3D::crossProduct(eyeVector, u);
    u = QVector3D::crossProduct(v, eyeVector);
    u.normalize();
    v.normalize();

    QQuaternion qRotation = QQuaternion::fromAxisAndAngle(u, rotation.x() * scale) *
                            QQuaternion::fromAxisAndAngle(v, rotation.y() * scale);

    QMatrix4x4 rotationMatrix;
    rotationMatrix.setToIdentity();
    rotationMatrix.rotate(qRotation);

    QVector3D newLightDir = QVector3D(rotationMatrix * QVector4D(QVector3D(
                                                                     light.direction), 1.0f));
    newLightDir.normalize();
    light.direction = QVector4D(newLightDir, 0.0);

    glBindBuffer(GL_UNIFORM_BUFFER, UBOLight);
    glBufferData(GL_UNIFORM_BUFFER, light.getStructSize(),
                 &light, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    lightViewMatrix.setToIdentity();
    lightViewMatrix.lookAt(-50 * QVector3D(light.direction), QVector3D(0, 0, 0), QVector3D(0,
                                                                                           0,
                                                                                           -1));
    shadowMatrix = lightProjectionMatrix * lightViewMatrix;

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 3 * SIZE_OF_MAT4, SIZE_OF_MAT4,
                    shadowMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    QVector3D lightPoint1(-newLightDir * 1000.0);
    QVector3D lightPoint2(newLightDir * 1000.0);
    vboLight.bind();
    vboLight.write(0, &lightPoint1, sizeof(GLfloat) * 3);
    vboLight.write(sizeof(GLfloat) * 3, &lightPoint2, sizeof(GLfloat) * 3);
    vboLight.release();
}

//------------------------------------------------------------------------------------------
void Renderer::renderTestScene()
{
    if(!initializedTestScene)
    {
        initTestScene();
        return;
    }

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(testProgram->programId());
    glBindVertexArray (vao);
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays (GL_TRIANGLES, 0, 3);
    glUseProgram(0);
}

//------------------------------------------------------------------------------------------
void Renderer::renderScene()
{
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(enabledRenderLight)
    {
        renderLight();
    }

    renderObjects(currentShadowMode == SHADOW_MAP);
}

//------------------------------------------------------------------------------------------
void Renderer::renderObjects(bool _withShadow)
{
    if(_withShadow)
    {
        if(!initializedDepthBuffer)
        {
            initDepthBufferObject();
        }

        generateShadowMap();

        /////////////////////////////////////////////////////////////////
        // render scene with shadow map
        makeCurrent();
        glViewport(0, 0, width() * retinaScale, height() * retinaScale);
        depthTexture->bind(2);
    }

    QOpenGLShaderProgram* program = glslPrograms[PhongShading];
    program->bind();
    program->setUniformValue(uniCameraPosition[PhongShading],
                             cameraPosition);
    program->setUniformValue(uniObjTexture[PhongShading], 0);
    program->setUniformValue(uniNormalTexture[PhongShading], 1);
    program->setUniformValue(uniDepthTexture[PhongShading], 2);
    program->setUniformValue(uniHasDepthTexture[PhongShading],
                             (GLboolean) _withShadow);
    program->setUniformValue(uniAmbientLight[PhongShading], ambientLight);

    glUniformBlockBinding(program->programId(), uniMatrices[PhongShading],
                          UBOBindingIndex[BINDING_MATRICES]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MATRICES],
                     UBOMatrices);
    glUniformBlockBinding(program->programId(), uniLight[PhongShading],
                          UBOBindingIndex[BINDING_LIGHT]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_LIGHT],
                     UBOLight);

    renderRoom(program);
    renderCube(program);

    if(currentShadingMode == PhongShading)
    {
        renderSphere(program);
        renderMeshObject(program);

        program->release();
    }


    if(_withShadow)
    {
        depthTexture->release();
    }



}

//------------------------------------------------------------------------------------------
void Renderer::generateShadowMap()
{
    /////////////////////////////////////////////////////////////////
    // render scene to shadow map
    FBODepthMap->bind();
    glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
    glDrawBuffer(GL_NONE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(4.0f, 4.0f);

    QOpenGLShaderProgram* shadowMapProgram = glslPrograms[ShadowMapShading];
    shadowMapProgram->bind();
    glUniformBlockBinding(shadowMapProgram->programId(),
                          uniMatrices[ShadowMapShading],
                          UBOBindingIndex[BINDING_MATRICES]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MATRICES],
                     UBOMatrices);
    shadowMapProgram->setUniformValue(uniHasObjTexture[ShadowMapShading], GL_FALSE);
    shadowMapProgram->setUniformValue(uniObjTexture[ShadowMapShading], 0);

    renderCube2DepthMap();
    renderSphere2DepthMap();
    renderMeshObject2DepthMap();

    glDisable(GL_POLYGON_OFFSET_FILL);
    shadowMapProgram->release();

    FBODepthMap->release();
}

//------------------------------------------------------------------------------------------
void Renderer::renderLight()
{
    if(!vaoLight.isCreated())
    {
        qDebug() << "vaoLight is not created!";
        return;
    }

    QOpenGLShaderProgram* program = glslPrograms[ProgramRenderLight];

    program->bind();

    /////////////////////////////////////////////////////////////////
    // set the uniform
    glUniformBlockBinding(program->programId(), uniMatrices[ProgramRenderLight],
                          UBOBindingIndex[BINDING_MATRICES]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MATRICES],
                     UBOMatrices);

    vaoLight.bind();
    glDrawArrays(GL_LINES, 0, 2);

    vaoLight.release();
    program->release();

}

//------------------------------------------------------------------------------------------
void Renderer::renderRoom(QOpenGLShaderProgram* _program)
{
    if(!vaoRoom[PhongShading].isCreated())
    {
        qDebug() << "vaoRoom is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    roomModelMatrix.constData());
    glBufferSubData(GL_UNIFORM_BUFFER, SIZE_OF_MAT4, SIZE_OF_MAT4,
                    roomNormalMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /////////////////////////////////////////////////////////////////
    // set the uniform

    glUniformBlockBinding(_program->programId(), uniMaterial[PhongShading],
                          UBOBindingIndex[BINDING_ROOM_MATERIAL]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_ROOM_MATERIAL],
                     UBORoomMaterial);
    _program->setUniformValue(uniHasObjTexture[PhongShading], GL_TRUE);
    _program->setUniformValue(uniHasNormalTexture[PhongShading], GL_TRUE);
    _program->setUniformValue(uniNeedTangent[PhongShading], GL_FALSE);

    /////////////////////////////////////////////////////////////////
    // render the room
    vaoRoom[PhongShading].bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    colorMapsWall[currentWallTexture]->bind(0);
    normalMapsWall[currentWallTexture]->bind(1);

    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);


    // 4 sides
    glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_SHORT, 0);
    // ceiling
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(GLushort) * 30));

    normalMapsWall[currentWallTexture]->release();
    colorMapsWall[currentWallTexture]->release();
    glDisable(GL_CULL_FACE);

    // floor
    if(currentFloorTexture != DunePattern)
    {
        colorMapsFloor[currentFloorTexture]->bind(0);
    }
    else
    {
        _program->setUniformValue(uniHasObjTexture[PhongShading], GL_FALSE);
    }

    normalMapsFloor[currentFloorTexture]->bind(1);

    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(GLushort) * 24));
    normalMapsFloor[currentFloorTexture]->release();

    if(currentFloorTexture != DunePattern)
    {
        colorMapsFloor[currentFloorTexture]->release();
    }

    vaoRoom[PhongShading].release();
}


//------------------------------------------------------------------------------------------
void Renderer::renderRoom2DepthMap()
{
    if(!vaoRoom[PhongShading].isCreated())
    {
        qDebug() << "vaoRoom is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    roomModelMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    vaoRoom[ShadowMapShading].bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDrawElements(GL_TRIANGLES, cubeObject->getNumIndices(), GL_UNSIGNED_SHORT, 0);
    glDisable(GL_CULL_FACE);
    vaoRoom[ShadowMapShading].release();
}


//------------------------------------------------------------------------------------------
void Renderer::renderCube(QOpenGLShaderProgram* _program)
{
    if(!vaoCube[PhongShading].isCreated())
    {
        qDebug() << "vaoCube is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    cubeModelMatrix.constData());
    glBufferSubData(GL_UNIFORM_BUFFER, SIZE_OF_MAT4, SIZE_OF_MAT4,
                    cubeNormalMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /////////////////////////////////////////////////////////////////
    // set the uniform
    _program->setUniformValue(uniHasObjTexture[PhongShading], GL_TRUE);
    _program->setUniformValue(uniHasNormalTexture[PhongShading], GL_FALSE);
    _program->setUniformValue(uniNeedTangent[PhongShading], GL_FALSE);
    _program->setUniformValue("discardTransparentPixel", GL_FALSE);
    glUniformBlockBinding(_program->programId(), uniMaterial[PhongShading],
                          UBOBindingIndex[BINDING_CUBE_MATERIAL]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_CUBE_MATERIAL],
                     UBOCubeMaterial);

    /////////////////////////////////////////////////////////////////
    // render the cube
    vaoCube[PhongShading].bind();
    decalTexture->bind(0);
    glDrawElements(GL_TRIANGLES, cubeObject->getNumIndices(), GL_UNSIGNED_SHORT, 0);
    decalTexture->release();
    vaoCube[PhongShading].release();
}

//------------------------------------------------------------------------------------------
void Renderer::renderCube2DepthMap()
{
    if(!vaoCube[ShadowMapShading].isCreated())
    {
        qDebug() << "vaoCube is not created!";
        return;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    cubeModelMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    vaoCube[ShadowMapShading].bind();
    glDrawElements(GL_TRIANGLES, cubeObject->getNumIndices(), GL_UNSIGNED_SHORT, 0);
    vaoCube[ShadowMapShading].release();
}

//------------------------------------------------------------------------------------------
void Renderer::renderSphere(QOpenGLShaderProgram* _program)
{
    if(!vaoSphere[currentShadingMode].isCreated())
    {
        qDebug() << "vaoSphere is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    sphereModelMatrix.constData());
    glBufferSubData(GL_UNIFORM_BUFFER, SIZE_OF_MAT4, SIZE_OF_MAT4,
                    sphereNormalMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformBlockBinding(_program->programId(), uniMaterial[currentShadingMode],
                          UBOBindingIndex[BINDING_SPHERE_MATERIAL]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_SPHERE_MATERIAL],
                     UBOSphereMaterial);


    /////////////////////////////////////////////////////////////////
    // set the uniform
    _program->setUniformValue(uniHasObjTexture[currentShadingMode], GL_TRUE);
    _program->setUniformValue(uniHasNormalTexture[currentShadingMode], GL_TRUE);
    _program->setUniformValue(uniNeedTangent[currentShadingMode], GL_TRUE);

    /////////////////////////////////////////////////////////////////
    // render the sphere
    vaoSphere[currentShadingMode].bind();
    colorMapsSphere->bind(0);
    normalMapsSphere->bind(1);
    glDrawElements(GL_TRIANGLES, sphereObject->getNumIndices(), GL_UNSIGNED_SHORT, 0);
    normalMapsSphere->release();
    colorMapsSphere->release();
    vaoSphere[currentShadingMode].release();
}


//------------------------------------------------------------------------------------------
void Renderer::renderSphere2DepthMap()
{
    if(!vaoSphere[currentShadingMode].isCreated())
    {
        qDebug() << "vaoSphere is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    sphereModelMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    /////////////////////////////////////////////////////////////////
    // render the sphere
    vaoSphere[ShadowMapShading].bind();
    glDrawElements(GL_TRIANGLES, sphereObject->getNumIndices(), GL_UNSIGNED_SHORT, 0);
    vaoSphere[ShadowMapShading].release();
}

//------------------------------------------------------------------------------------------
void Renderer::renderMeshObject(QOpenGLShaderProgram* _program)
{
    if(!vaoMeshObject[currentShadingMode].isCreated())
    {
        qDebug() << "vaoMeshObject is not created!";
        return;
    }

    /////////////////////////////////////////////////////////////////
    // flush the model and normal matrices
    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    meshObjectModelMatrix.constData());
    glBufferSubData(GL_UNIFORM_BUFFER, SIZE_OF_MAT4, SIZE_OF_MAT4,
                    meshObjectNormalMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformBlockBinding(_program->programId(), uniMaterial[currentShadingMode],
                          UBOBindingIndex[BINDING_MESH_OBJECT_MATERIAL]);
    glBindBufferBase(GL_UNIFORM_BUFFER, UBOBindingIndex[BINDING_MESH_OBJECT_MATERIAL],
                     UBOMeshObjectMaterial);


    /////////////////////////////////////////////////////////////////
    // set the uniform
    _program->setUniformValue(uniHasObjTexture[currentShadingMode], GL_TRUE);
    _program->setUniformValue(uniHasNormalTexture[currentShadingMode], GL_TRUE);
    _program->setUniformValue(uniNeedTangent[currentShadingMode], GL_TRUE);

    /////////////////////////////////////////////////////////////////
    // render the mesh object
    vaoMeshObject[currentShadingMode].bind();
    colorMapsMeshObject[currentMeshObjectTexture]->bind(0);
    normalMapsMeshObject[currentMeshObjectTexture]->bind(1);
    glDrawArrays(GL_TRIANGLES, 0, objLoader->getNumVertices());
    normalMapsMeshObject[currentMeshObjectTexture]->release();
    colorMapsMeshObject[currentMeshObjectTexture]->release();
    vaoMeshObject[currentShadingMode].release();

}


//------------------------------------------------------------------------------------------
void Renderer::renderMeshObject2DepthMap()
{
    if(!vaoMeshObject[ShadowMapShading].isCreated())
    {
        qDebug() << "vaoMeshObject is not created!";
        return;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, SIZE_OF_MAT4,
                    meshObjectModelMatrix.constData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    vaoMeshObject[ShadowMapShading].bind();
    glDrawArrays(GL_TRIANGLES, 0, objLoader->getNumVertices());
    vaoMeshObject[ShadowMapShading].release();
}


