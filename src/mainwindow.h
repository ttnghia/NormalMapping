//------------------------------------------------------------------------------------------
// mainwindow.h
//
// Created on: 1/17/2015
//     Author: Nghia Truong
//
//------------------------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSPinbox>
#include <QtGui>
#include <QtWidgets>

#include "renderer.h"
#include "colorselector.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

protected:
    void keyPressEvent(QKeyEvent*);

    void setupGUI();

public slots:
    void resetObjectPositions();
    void changeMouseTransformTarget(bool _state);
    void changeShadowMethod(bool _state);
    void prevFloorTexture();
    void nextFloorTexture();
    void prevWallTexture();
    void nextWallTexture();
    void prevMeshObjectTexture();
    void nextMeshObjectTexture();
    void prevMeshObject();
    void nextMeshObject();

private:
    Renderer* renderer;

    QComboBox* cbMeshObject;

    QMap<QRadioButton*, ShadowModes> rdb2ShadowMethodMap;

    ColorSelector* wgCubeColor;
    QSlider* sldRoomSize;

    QMap<QRadioButton*, MouseTransformationTarget> rdb2MouseTransTargetMap;

    QComboBox* cbFloorTexture;
    QComboBox* cbWallTexture;
    QComboBox* cbMeshObjectTexture;

};

#endif // MAINWINDOW_H
