//------------------------------------------------------------------------------------------
// mainwindow.cpp
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("Shader-based Normal Mapping");

    setupGUI();

    // Update continuously
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), renderer, SLOT(update()));
    timer->start(10);
}

//------------------------------------------------------------------------------------------
QSize MainWindow::sizeHint() const
{
    return QSize(1600, 1200);
}

//------------------------------------------------------------------------------------------
QSize MainWindow::minimumSizeHint() const
{
    return QSize(50, 50);
}

//------------------------------------------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
    case Qt::Key_Escape:
        close();
        break;

    case Qt::Key_R:
        renderer->resetCameraPosition();
        break;

    case Qt::Key_F:
        nextFloorTexture();
        break;

    case Qt::Key_W:
        nextWallTexture();
        break;

    case Qt::Key_M:
        nextMeshObjectTexture();
        break;

    default:
        renderer->keyPressEvent(e);
    }
}


//------------------------------------------------------------------------------------------
void MainWindow::setupGUI()
{
    renderer = new Renderer(this);

    ////////////////////////////////////////////////////////////////////////////////
    // floor textures
    QGridLayout* floorTextureLayout = new QGridLayout;
    cbFloorTexture = new QComboBox;
    floorTextureLayout->addWidget(cbFloorTexture, 0, 0, 1, 3);

    QToolButton* btnPreviousFloorTexture = new QToolButton;
    btnPreviousFloorTexture->setArrowType(Qt::LeftArrow);
    floorTextureLayout->addWidget(btnPreviousFloorTexture, 0, 3, 1, 1);

    QToolButton* btnNextFloorTexture = new QToolButton;
    btnNextFloorTexture->setArrowType(Qt::RightArrow);
    floorTextureLayout->addWidget(btnNextFloorTexture, 0, 4, 1, 1);

    QStringList* strListFloorTexture = renderer->getStrListFloorTexture();

    for(int i = 0; i < strListFloorTexture->size(); ++i)
    {
        cbFloorTexture->addItem(strListFloorTexture->at(i));
    }

    QGroupBox* floorTextureGroup = new QGroupBox("Floor Bump Map Texture");
    floorTextureGroup->setLayout(floorTextureLayout);

    connect(cbFloorTexture, SIGNAL(currentIndexChanged(int)), renderer,
            SLOT(setFloorTexture(int)));
    connect(btnPreviousFloorTexture, SIGNAL(clicked()), this, SLOT(prevFloorTexture()));
    connect(btnNextFloorTexture, SIGNAL(clicked()), this, SLOT(nextFloorTexture()));

    ////////////////////////////////////////////////////////////////////////////////
    // wall textures
    QGridLayout* wallTextureLayout = new QGridLayout;

    cbWallTexture = new QComboBox;
    wallTextureLayout->addWidget(cbWallTexture, 0, 0, 1, 3);

    QToolButton* btnPreviousWallTexture = new QToolButton;
    btnPreviousWallTexture->setArrowType(Qt::LeftArrow);
    wallTextureLayout->addWidget(btnPreviousWallTexture, 0, 3, 1, 1);

    QToolButton* btnNextWallTexture = new QToolButton;
    btnNextWallTexture->setArrowType(Qt::RightArrow);
    wallTextureLayout->addWidget(btnNextWallTexture, 0, 4, 1, 1);

    QStringList* strListWallTexture = renderer->getStrListWallTexture();

    for(int i = 0; i < strListWallTexture->size(); ++i)
    {
        cbWallTexture->addItem(strListWallTexture->at(i));
    }


    QGroupBox* wallTextureGroup = new QGroupBox("Wall Bump Map Texture");
    wallTextureGroup->setLayout(wallTextureLayout);

    connect(cbWallTexture, SIGNAL(currentIndexChanged(int)), renderer,
            SLOT(setWallTexture(int)));
    connect(btnPreviousWallTexture, SIGNAL(clicked()), this, SLOT(prevWallTexture()));
    connect(btnNextWallTexture, SIGNAL(clicked()), this, SLOT(nextWallTexture()));

    ////////////////////////////////////////////////////////////////////////////////
    // mesh object textures
    QGridLayout* meshObjectTextureLayout = new QGridLayout;

    cbMeshObjectTexture = new QComboBox;
    meshObjectTextureLayout->addWidget(cbMeshObjectTexture, 0, 0, 1, 3);

    QToolButton* btnPreviousMeshObjectTexture = new QToolButton;
    btnPreviousMeshObjectTexture->setArrowType(Qt::LeftArrow);
    meshObjectTextureLayout->addWidget(btnPreviousMeshObjectTexture, 0, 3, 1, 1);

    QToolButton* btnNextMeshObjectTexture = new QToolButton;
    btnNextMeshObjectTexture->setArrowType(Qt::RightArrow);
    meshObjectTextureLayout->addWidget(btnNextMeshObjectTexture, 0, 4, 1, 1);

    QStringList* strListMeshObjectTexture = renderer->getStrListMeshObjectTexture();

    for(int i = 0; i < strListMeshObjectTexture->size(); ++i)
    {
        cbMeshObjectTexture->addItem(strListMeshObjectTexture->at(i));
    }

    cbMeshObjectTexture->setCurrentIndex(MetalTexture::CopperVerdigris);

    QGroupBox* meshObjectTextureGroup = new QGroupBox("Mesh Object Bump Map Texture");
    meshObjectTextureGroup->setLayout(meshObjectTextureLayout);

    connect(cbMeshObjectTexture, SIGNAL(currentIndexChanged(int)), renderer,
            SLOT(setMeshObjectTexture(int)));
    connect(btnPreviousMeshObjectTexture, SIGNAL(clicked()), this,
            SLOT(prevMeshObjectTexture()));
    connect(btnNextMeshObjectTexture, SIGNAL(clicked()), this, SLOT(nextMeshObjectTexture()));

    ////////////////////////////////////////////////////////////////////////////////
    // object color picker
    wgCubeColor = new ColorSelector;

    connect(wgCubeColor, &ColorSelector::colorChanged, renderer, &Renderer::setCubeColor);

    wgCubeColor->setColor(QColor(170, 85, 255));


    QGridLayout* objectColorLayout = new QGridLayout;
    objectColorLayout->addWidget(new QLabel("Cube:"), 0, 0, Qt::AlignRight);
    objectColorLayout->addWidget(wgCubeColor, 0, 1, 1, 2);

    QGroupBox* objectColorGroup = new QGroupBox("Object Color");
    objectColorGroup->setLayout(objectColorLayout);

    ////////////////////////////////////////////////////////////////////////////////
    // mesh object
    cbMeshObject = new QComboBox;
    QString str;
    str = QString("Teapot");
    cbMeshObject->addItem(str);

    str = QString("Bunny");
    cbMeshObject->addItem(str);
    QGridLayout* meshObjectLayout = new QGridLayout;
    meshObjectLayout->addWidget(cbMeshObject, 0, 0, 1, 3);
    QGroupBox* meshObjectGroup = new QGroupBox("Mesh Object");
    meshObjectGroup->setLayout(meshObjectLayout);

    QToolButton* btnPreviousMeshObject = new QToolButton;
    btnPreviousMeshObject->setArrowType(Qt::LeftArrow);
    meshObjectLayout->addWidget(btnPreviousMeshObject, 0, 3, 1, 1);

    QToolButton* btnNextMeshObject = new QToolButton;
    btnNextMeshObject->setArrowType(Qt::RightArrow);
    meshObjectLayout->addWidget(btnNextMeshObject, 0, 4, 1, 1);


    cbMeshObject->setCurrentIndex(MeshObject::BUNNY_OBJ);
    connect(cbMeshObject, SIGNAL(currentIndexChanged(int)), renderer,
            SLOT(setMeshObject(int)));
    connect(btnPreviousMeshObject, SIGNAL(clicked()), this,
            SLOT(prevMeshObject()));
    connect(btnNextMeshObject, SIGNAL(clicked()), this, SLOT(nextMeshObject()));

    ////////////////////////////////////////////////////////////////////////////////
    // room size
    sldRoomSize = new QSlider(Qt::Horizontal);
    sldRoomSize->setMinimum(3);
    sldRoomSize->setMaximum(100);
    sldRoomSize->setValue(3);

    connect(sldRoomSize, &QSlider::valueChanged, renderer,
            &Renderer::setRoomSize);

    QVBoxLayout* roomSizeLayout = new QVBoxLayout;
    roomSizeLayout->addWidget(sldRoomSize);
    QGroupBox* roomSizeGroup = new QGroupBox("Room Size");
    roomSizeGroup->setLayout(roomSizeLayout);


    ////////////////////////////////////////////////////////////////////////////////
    // light intensity
    QSlider* sldLightIntensity = new QSlider(Qt::Horizontal);
    sldLightIntensity->setMinimum(0);
    sldLightIntensity->setMaximum(100);
    sldLightIntensity->setValue(80);

    connect(sldLightIntensity, &QSlider::valueChanged, renderer,
            &Renderer::setLightIntensity);



    QSlider* sldAmbientLight = new QSlider(Qt::Horizontal);
    sldAmbientLight->setMinimum(0);
    sldAmbientLight->setMaximum(100);
    sldAmbientLight->setValue(30);

    connect(sldAmbientLight, &QSlider::valueChanged, renderer,
            &Renderer::setAmbientLightIntensity);


    QSlider* sldSpecularReflection = new QSlider(Qt::Horizontal);
    sldSpecularReflection->setMinimum(0);
    sldSpecularReflection->setMaximum(100);
    sldSpecularReflection->setValue(50);

    connect(sldSpecularReflection, &QSlider::valueChanged, renderer,
            &Renderer::setObjectsSpecularReflection);

    QGridLayout* lightIntensityLayout = new QGridLayout;
    lightIntensityLayout->addWidget(new QLabel("Directional:"), 0, 0);
    lightIntensityLayout->addWidget(sldLightIntensity, 0, 1);
    lightIntensityLayout->addWidget(new QLabel("Ambient:"), 1, 0);
    lightIntensityLayout->addWidget(sldAmbientLight, 1, 1);
    lightIntensityLayout->addWidget(new QLabel("Specular:"), 2, 0);
    lightIntensityLayout->addWidget(sldSpecularReflection, 2, 1);

    QGroupBox* lightIntensityGroup = new QGroupBox("All Objects' Light Intensity");
    lightIntensityGroup->setLayout(lightIntensityLayout);

    ////////////////////////////////////////////////////////////////////////////////
    // shadow generation
    QRadioButton* rdbNoShadow = new QRadioButton("No Shadow");
    rdb2ShadowMethodMap[rdbNoShadow] = NO_SHADOW;

    QRadioButton* rdbShadowMap = new QRadioButton("Shadow Map");
    rdb2ShadowMethodMap[rdbShadowMap] = SHADOW_MAP;
    rdbShadowMap->setChecked(true);

    QGridLayout* shadowLayout = new QGridLayout;
    shadowLayout->addWidget(rdbNoShadow, 0, 0);
    shadowLayout->addWidget(rdbShadowMap, 0, 1);

    QGroupBox* shadowGroup = new QGroupBox("Shadow Generation");
    shadowGroup->setLayout(shadowLayout);


    connect(rdbNoShadow, &QRadioButton::toggled, this,
            &MainWindow::changeShadowMethod);
    connect(rdbShadowMap, &QRadioButton::toggled, this,
            &MainWindow::changeShadowMethod);

    ////////////////////////////////////////////////////////////////////////////////
    // mouse drag transformation
    QRadioButton* rdbMoveCamera;
    QRadioButton* rdbMoveCubeWithSphere;
    QRadioButton* rdbMoveLight;
    QRadioButton* rdbMoveMeshObject;

    rdbMoveCamera = new QRadioButton("Camera");
    rdbMoveCamera->setChecked(true);
    rdb2MouseTransTargetMap[rdbMoveCamera] = TRANSFORM_CAMERA;
    connect(rdbMoveCamera, &QRadioButton::clicked, this,
            &MainWindow::changeMouseTransformTarget);

    rdbMoveCubeWithSphere = new QRadioButton(
        QString::fromUtf8("Cube && Sphere"));
    rdb2MouseTransTargetMap[rdbMoveCubeWithSphere] = TRANSFORM_CUBE_SPHERE;
    connect(rdbMoveCubeWithSphere, &QRadioButton::clicked, this,
            &MainWindow::changeMouseTransformTarget);

    rdbMoveLight = new QRadioButton( QString::fromUtf8("Light Source"));
    rdbMoveLight->setChecked(false);
    rdb2MouseTransTargetMap[rdbMoveLight] = TRANSFORM_LIGHT;
    connect(rdbMoveLight, &QRadioButton::toggled, this,
            &MainWindow::changeMouseTransformTarget);

    rdbMoveMeshObject = new QRadioButton(
        QString::fromUtf8("Mesh Object"));
    rdb2MouseTransTargetMap[rdbMoveMeshObject] = TRANSFORM_MESH_OBJECT;
    rdbMoveMeshObject->setChecked(false);
    connect(rdbMoveMeshObject, &QRadioButton::clicked, this,
            &MainWindow::changeMouseTransformTarget);



    QGridLayout* mouseTransformationTargetLayout = new QGridLayout;
    mouseTransformationTargetLayout->addWidget(rdbMoveCamera, 0, 0);
    mouseTransformationTargetLayout->addWidget(rdbMoveCubeWithSphere, 0, 1);
    mouseTransformationTargetLayout->addWidget(rdbMoveLight, 1, 0);
    mouseTransformationTargetLayout->addWidget(rdbMoveMeshObject, 1, 1);

    QGroupBox* mouseTransformationTargetGroup = new QGroupBox("Mouse Transformation Target");
    mouseTransformationTargetGroup->setLayout(mouseTransformationTargetLayout);


    ////////////////////////////////////////////////////////////////////////////////
    // others
    QCheckBox* chkRenderLight = new QCheckBox("Show Light Direction");
    connect(chkRenderLight, &QCheckBox::toggled, renderer, &Renderer::enableRenderLight);
    connect(rdbMoveLight, &QRadioButton::toggled, chkRenderLight, &QCheckBox::setChecked);

    QPushButton* btnResetObjects = new QPushButton("Reset Object Positions");
    connect(btnResetObjects, SIGNAL(clicked()), this,
            SLOT(resetObjectPositions()));

    QPushButton* btnResetCamera = new QPushButton("Reset Camera");
    connect(btnResetCamera, &QPushButton::clicked, renderer,
            &Renderer::resetCameraPosition);


    ////////////////////////////////////////////////////////////////////////////////
    // Add slider group to parameter group
    QVBoxLayout* parameterLayout = new QVBoxLayout;
    parameterLayout->addWidget(floorTextureGroup);
    parameterLayout->addWidget(wallTextureGroup);
    parameterLayout->addWidget(meshObjectTextureGroup);
    parameterLayout->addWidget(meshObjectGroup);
    parameterLayout->addWidget(objectColorGroup);
    parameterLayout->addWidget(roomSizeGroup);
    parameterLayout->addWidget(lightIntensityGroup);
    parameterLayout->addWidget(shadowGroup);
    parameterLayout->addWidget(mouseTransformationTargetGroup);
    parameterLayout->addWidget(chkRenderLight);

    parameterLayout->addWidget(btnResetObjects);
    parameterLayout->addWidget(btnResetCamera);


    parameterLayout->addStretch();

    QGroupBox* parameterGroup = new QGroupBox;
    parameterGroup->setFixedWidth(300);
    parameterGroup->setLayout(parameterLayout);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addWidget(renderer);
    hLayout->addWidget(parameterGroup);

    setLayout(hLayout);

}

//------------------------------------------------------------------------------------------
void MainWindow::resetObjectPositions()
{
    sldRoomSize->setValue(3);
    renderer->resetObjectPositions();
    renderer->resetLightDirection();
}

//------------------------------------------------------------------------------------------
void MainWindow::changeMouseTransformTarget(bool _state)
{
    if(!_state)
    {
        return;
    }

    QRadioButton* rdbTransformTarget = qobject_cast<QRadioButton*>(QObject::sender());

    if(!rdbTransformTarget)
    {
        return;
    }


    renderer->setMouseTransformationTarget(rdb2MouseTransTargetMap.value(rdbTransformTarget));

}

//------------------------------------------------------------------------------------------
void MainWindow::changeShadowMethod(bool _state)
{
    if(!_state)
    {
        return;
    }

    QRadioButton* rdbShadowMethod = qobject_cast<QRadioButton*>(QObject::sender());

    if(!rdbShadowMethod)
    {
        return;
    }

    renderer->setShadowMethod(rdb2ShadowMethodMap[rdbShadowMethod]);
}

//------------------------------------------------------------------------------------------
void MainWindow::prevFloorTexture()
{
    int index = cbFloorTexture->currentIndex();

    if(index > 0)
    {
        cbFloorTexture->setCurrentIndex(index - 1);
    }
    else
    {
        cbFloorTexture->setCurrentIndex(cbFloorTexture->count() - 1);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::nextFloorTexture()
{
    int index = cbFloorTexture->currentIndex();

    if(index < cbFloorTexture->count() - 1)
    {
        cbFloorTexture->setCurrentIndex(index + 1);
    }
    else
    {
        cbFloorTexture->setCurrentIndex(0);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::prevWallTexture()
{
    int index = cbWallTexture->currentIndex();

    if(index > 0)
    {
        cbWallTexture->setCurrentIndex(index - 1);
    }
    else
    {
        cbWallTexture->setCurrentIndex(cbWallTexture->count() - 1);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::nextWallTexture()
{
    int index = cbWallTexture->currentIndex();

    if(index < cbWallTexture->count() - 1)
    {
        cbWallTexture->setCurrentIndex(index + 1);
    }
    else
    {
        cbWallTexture->setCurrentIndex(0);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::prevMeshObjectTexture()
{
    int index = cbMeshObjectTexture->currentIndex();

    if(index > 0)
    {
        cbMeshObjectTexture->setCurrentIndex(index - 1);
    }
    else
    {
        cbMeshObjectTexture->setCurrentIndex(cbMeshObjectTexture->count() - 1);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::nextMeshObjectTexture()
{
    int index = cbMeshObjectTexture->currentIndex();

    if(index < cbMeshObjectTexture->count() - 1)
    {
        cbMeshObjectTexture->setCurrentIndex(index + 1);
    }
    else
    {
        cbMeshObjectTexture->setCurrentIndex(0);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::prevMeshObject()
{
    int index = cbMeshObject->currentIndex();

    if(index > 0)
    {
        cbMeshObject->setCurrentIndex(index - 1);
    }
    else
    {
        cbMeshObject->setCurrentIndex(cbMeshObject->count() - 1);
    }
}

//------------------------------------------------------------------------------------------
void MainWindow::nextMeshObject()
{
    int index = cbMeshObject->currentIndex();

    if(index < cbMeshObject->count() - 1)
    {
        cbMeshObject->setCurrentIndex(index + 1);
    }
    else
    {
        cbMeshObject->setCurrentIndex(0);
    }
}
