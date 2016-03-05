//------------------------------------------------------------------------------------------
//
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------

#ifndef GLRENDERER_H
#define GLRENDERER_H

#include <QtGui>
#include <QtWidgets>
#include <QOpenGLFunctions_4_0_Core>

#include "unitcube.h"
#include "unitsphere.h"
#include "unitplane.h"
#include "objloader.h"

//------------------------------------------------------------------------------------------
#define PRINT_LINE \
{ \
    qDebug()<< "Line:" << __LINE__ << ", file:" << __FILE__; \
}

#define PRINT_ERROR(_errStr) \
{ \
    qDebug()<< "Error occured at line:" << __LINE__ << ", file:" << __FILE__; \
    qDebug()<< "Error message:" << _errStr; \
}

#define PRINT_AND_DIE(_errStr) \
{ \
    qDebug()<< "Error occured at line:" << __LINE__ << ", file:" << __FILE__; \
    qDebug()<< "Error message:" << _errStr; \
    exit(EXIT_FAILURE); \
}

#define TRUE_OR_DIE(_condition, _errStr) \
{ \
    if(!(_condition)) \
    { \
        qDebug()<< "Fatal error occured at line:" << __LINE__ << ", file:" << __FILE__; \
        qDebug()<< "Error message:" << _errStr; \
        exit(EXIT_FAILURE); \
    } \
}

#define SIZE_OF_MAT4 (4 * 4 *sizeof(GLfloat))
#define SIZE_OF_VEC4 (4 * sizeof(GLfloat))
//------------------------------------------------------------------------------------------
#define MOVING_INERTIA 0.9f
#define DEPTH_TEXTURE_SIZE 1024
#define DEFAULT_CAMERA_POSITION QVector3D(0.0f,  6.5f, 25.0f)
#define DEFAULT_CAMERA_FOCUS QVector3D(0.0f,  6.5f, 0.0f)
#define DEFAULT_LIGHT_DIRECTION QVector4D(1.0f, -1.0f, -1.0f, 1.0f)
#define DEFAULT_CUBE_POSITION QVector3D(3.0f, 1.001f, 1.0f)
#define DEFAULT_SPHERE_POSITION QVector3D(6.0f, 6.0f, 2.0f)
#define DEFAULT_MESH_OBJECT_POSITION QVector3D(-4.0f, 0.001f, -3.0f)

struct Light
{
    Light():
        direction(0.0f, -1.0f, 0.0f, 1.0f),
        color(1.0f, 1.0f, 1.0f, 1.0f),
        intensity(1.0f) {}

    int getStructSize()
    {
        return (2 * 4 + 1) * sizeof(GLfloat);
    }

    QVector4D direction;
    QVector4D color;
    GLfloat intensity;
};

struct Material
{
    Material():
        diffuseColor(-10.0f, 1.0f, 0.0f, 1.0f),
        specularColor(1.0f, 1.0f, 1.0f, 1.0f),
        reflection(0.0f),
        shininess(10.0f) {}

    int getStructSize()
    {
        return (2 * 4 + 2) * sizeof(GLfloat);
    }

    void setDiffuse(QVector4D _diffuse)
    {
        diffuseColor = _diffuse;
    }

    void setSpecular(QVector4D _specular)
    {
        specularColor = _specular;
    }

    void setReflection(float _reflection)
    {
        reflection = _reflection;
    }

    QVector4D diffuseColor;
    QVector4D specularColor;
    GLfloat reflection;
    GLfloat shininess;
};

enum FloorTexture
{
    DunePattern = 0,
    BlueMarblePersian,
    BlueMarbleSlabs,
    BrownByzantine,
    CorrodedTechnoTiles,
    NumFloorTextures
};

enum WallTexture
{
    AlienCarving = 0,
    AlternatingBrick,
    AncientMayanBlocks,
    BubblyBricks,
    CarvedSandstone,
    ChippedBricks,
    DesertBlushBrick,
    NumWallTextures
};

enum MetalTexture
{
    AluminumTubing = 0,
    BronzeAgeArtifact,
    CopperVerdigris,
    PyramidVerdigris,
    TexturedMetal,
    NumMetalTextures
};


enum MeshObject
{
    TEAPOT_OBJ = 0,
    BUNNY_OBJ,
    NUM_MESH_OBJECT
};

enum MouseTransformationTarget
{
    TRANSFORM_CAMERA = 0,
    TRANSFORM_CUBE_SPHERE,
    TRANSFORM_LIGHT,
    TRANSFORM_MESH_OBJECT,
    NUM_TRANSFORMATION_TARGET
};

enum ShadowModes
{
    NO_SHADOW = 0,
    SHADOW_MAP,
    NUM_SHADOW_METHODS
};


enum ShadingProgram
{
    PhongShading = 0,
    ShadowMapShading,
    ProgramRenderLight,
    NUM_PROGRAMS
};

enum UBOBinding
{
    BINDING_MATRICES = 0,
    BINDING_LIGHT,
    BINDING_ROOM_MATERIAL,
    BINDING_CUBE_MATERIAL,
    BINDING_SPHERE_MATERIAL,
    BINDING_MESH_OBJECT_MATERIAL,
    NUM_BINDING_POINTS
};


//------------------------------------------------------------------------------------------
class Renderer : public QOpenGLWidget, QOpenGLFunctions_4_0_Core// QOpenGLFunctions
{
    Q_OBJECT
public:
    enum SpecialKey
    {
        NO_KEY,
        SHIFT_KEY,
        CTRL_KEY
    };

    enum MouseButton
    {
        NO_BUTTON,
        LEFT_BUTTON,
        RIGHT_BUTTON
    };

    Renderer(QWidget* parent = 0);
    ~Renderer();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void keyPressEvent(QKeyEvent* _event);
    void keyReleaseEvent(QKeyEvent* _event);
    void wheelEvent(QWheelEvent* _event);

    void setShadingMode(ShadingProgram _shadingMode);
    void setTextureFilteringMode(QOpenGLTexture::Filter _textureFiltering);

    QStringList* getStrListFloorTexture();
    QStringList* getStrListWallTexture();
    QStringList* getStrListMeshObjectTexture();

public slots:
    void enableDepthTest(bool _status);
    void enableRenderLight(bool _state);
    void setMouseTransformationTarget(MouseTransformationTarget _mouseTarget);
    void setShadowMethod(ShadowModes _shadowMode = NO_SHADOW);
    void setRoomSize(int _roomSize);
    void setAmbientLightIntensity(int _ambientLight);
    void setLightIntensity(int _intensity);
    void setObjectsSpecularReflection(int _intensity);
    void setMeshObject(int _objectIndex);
    void setCubeColor(float _r, float _g, float _b);
    void setMeshObjectColor(float _r, float _g, float _b);
    void setFloorTexture(int _texture);
    void setWallTexture(int _texture);
    void setMeshObjectTexture(int _texture);

    void resetCameraPosition();
    void resetObjectPositions();
    void resetLightDirection();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent* _event);
    void mouseMoveEvent(QMouseEvent* _event);
    void mouseReleaseEvent(QMouseEvent* _event);

private:
    void checkOpenGLVersion();
    void initTestScene();
    void initScene();
    bool initShaderPrograms();
    bool validateShaderPrograms(ShadingProgram _shadingMode);
    bool initPhongShadingProgram();
    bool initShadowMapShadingProgram();
    bool initRenderLightProgram();

    void initSharedBlockUniform();
    void initTexture();
    void initSceneMemory();
    void initRoomMemory();
    void initCubeMemory();
    void initSphereMemory();
    void initMeshObjectMemory();
    void initLightObjectMemory();

    void initVertexArrayObjects();
    void initRoomVAO(ShadingProgram _shadingMode);
    void initCubeVAO(ShadingProgram _shadingMode);
    void initSphereVAO(ShadingProgram _shadingMode);
    void initMeshObjectVAO(ShadingProgram _shadingMode);
    void initLightVAO();
    void initSceneMatrices();
    void initDepthBufferObject();

    void updateCamera();
    void translateCamera();
    void rotateCamera();
    void zoomCamera();

    void translateCubeSphere();
    void rotateCubeSphere();

    void translateMeshObject();
    void rotateMeshObject();

    void rotateLight();

    void renderTestScene();

    void renderScene();
    void renderObjects(bool _withShadow);
    void generateShadowMap();

    void renderLight();
    void renderRoom(QOpenGLShaderProgram* _program);
    void renderRoom2DepthMap();

    void renderCube(QOpenGLShaderProgram* _program);
    void renderCube2DepthMap();

    void renderSphere(QOpenGLShaderProgram* _program);
    void renderSphere2DepthMap();

    void renderMeshObject(QOpenGLShaderProgram* _program);
    void renderMeshObject2DepthMap();


    QOpenGLTexture* normalMapsFloor[NumFloorTextures];
    QOpenGLTexture* colorMapsFloor[NumFloorTextures];
    QOpenGLTexture* normalMapsWall[NumWallTextures];
    QOpenGLTexture* colorMapsWall[NumWallTextures];
    QOpenGLTexture* normalMapsMeshObject[NumMetalTextures];
    QOpenGLTexture* colorMapsMeshObject[NumMetalTextures];
    QOpenGLTexture* normalMapsSphere;
    QOpenGLTexture* colorMapsSphere;
    QStringList* strListFloorTexture;
    QStringList* strListWallTexture;
    QStringList* strListMeshObjectTexture;

    QOpenGLTexture* decalTexture;
    UnitCube* cubeObject;
    UnitSphere* sphereObject;
    OBJLoader* objLoader;

    QMap<ShadingProgram, QString> vertexShaderSourceMap;
    QMap<ShadingProgram, QString> fragmentShaderSourceMap;
    QOpenGLShaderProgram* glslPrograms[NUM_PROGRAMS];
    GLuint UBOBindingIndex[NUM_BINDING_POINTS];
    GLuint UBOMatrices;
    GLuint UBOLight;
    GLuint UBORoomMaterial;
    GLuint UBOCubeMaterial;
    GLuint UBOSphereMaterial;
    GLuint UBOMeshObjectMaterial;
    GLint attrVertex[NUM_PROGRAMS];
    GLint attrNormal[NUM_PROGRAMS];
    GLint attrTexCoord[NUM_PROGRAMS];

    GLint uniMatrices[NUM_PROGRAMS];
    GLint uniCameraPosition[NUM_PROGRAMS];
    GLint uniLight[NUM_PROGRAMS];
    GLint uniAmbientLight[NUM_PROGRAMS];
    GLint uniMaterial[NUM_PROGRAMS];
    GLint uniObjTexture[NUM_PROGRAMS];
    GLint uniNormalTexture[NUM_PROGRAMS];
    GLint uniDepthTexture[NUM_PROGRAMS];
    GLint uniHasObjTexture[NUM_PROGRAMS];
    GLint uniHasNormalTexture[NUM_PROGRAMS];
    GLint uniHasDepthTexture[NUM_PROGRAMS];
    GLint uniNeedTangent[NUM_PROGRAMS];
    GLint uniPlaneVector;
    GLint uniShadowIntensity;

    QOpenGLFramebufferObject* FBODepthMap;
    QOpenGLTexture* depthTexture;

    QOpenGLVertexArrayObject vaoRoom[NUM_PROGRAMS];
    QOpenGLVertexArrayObject vaoCube[NUM_PROGRAMS];
    QOpenGLVertexArrayObject vaoSphere[NUM_PROGRAMS];
    QOpenGLVertexArrayObject vaoMeshObject[NUM_PROGRAMS];
    QOpenGLVertexArrayObject vaoLight;

    QOpenGLBuffer vboLight;
    QOpenGLBuffer vboRoom;
    QOpenGLBuffer vboCube;
    QOpenGLBuffer vboSphere;
    QOpenGLBuffer vboMeshObject;
    QOpenGLBuffer iboRoom;
    QOpenGLBuffer iboCube;
    QOpenGLBuffer iboSphere;

    Material roomMaterial;
    Material cubeMaterial;
    Material sphereMaterial;
    Material meshObjectMaterial;
    Light light;


    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewProjectionMatrix;

    QMatrix4x4 roomModelMatrix;
    QMatrix4x4 roomNormalMatrix;
    QMatrix4x4 cubeModelMatrix;
    QMatrix4x4 cubeNormalMatrix;
    QMatrix4x4 sphereModelMatrix;
    QMatrix4x4 sphereNormalMatrix;
    QMatrix4x4 meshObjectModelMatrix;
    QMatrix4x4 meshObjectNormalMatrix;

    QMatrix4x4 lightViewMatrix;
    QMatrix4x4 lightProjectionMatrix;
    QMatrix4x4 shadowMatrix;

    qreal retinaScale;
    float zooming;
    QVector3D cameraPosition;
    QVector3D cameraFocus;
    QVector3D cameraUpDirection;

    QVector2D lastMousePos;
    QVector3D translation;
    QVector3D translationLag;
    QVector3D rotation;
    QVector3D rotationLag;
    QVector3D scalingLag;
    SpecialKey specialKeyPressed;
    MouseButton mouseButtonPressed;

    ShadingProgram currentShadingMode;
    int currentFloorTexture;
    int currentWallTexture;
    int currentMeshObjectTexture;
    MeshObject currentMeshObject;
    MouseTransformationTarget currentMouseTransTarget;
    ShadowModes currentShadowMode;
    float ambientLight;
    float roomSize;

    bool enabledRenderLight;

    bool initializedScene;
    bool initializedTestScene;
    bool initializedDepthBuffer;

};

#endif // GLRENDERER_H
