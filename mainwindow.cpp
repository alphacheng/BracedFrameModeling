/* *****************************************************************************
Copyright (c) 2018-2019, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */


#include "mainwindow.h"
 //#include "ui_mainwindow.h"

#include <iostream>

// layouts
#include <HeaderWidget.h>
#include <FooterWidget.h>
//#include "sectiontitle.h"

// custom
//#include <SimpleSpreadsheetWidget.h>
#include <qcustomplot/qcustomplot.h>
#include "experiment.h"
#include "resp.h"
#include "historywidget.h"
#include "deformwidget.h"
#include "responsewidget.h"
#include "hysteresiswidget.h"

// widget libraries
#include <QtGui>
#include <QtWidgets>
#include <QtCore>
#include <QDebug>
#include <QGuiApplication>

#include <Response.h>
#include <Information.h>
// other
#include <math.h>

// files
#include <QFile>
//#include <QtSql>
//#include <QMap>

//styles
//#include <QGroupBox>
//#include <QFrame>

// OpenSees include files
#include <Node.h>
#include <SP_Constraint.h>
#include <MP_Constraint.h>

// other
#include <Vector.h>
#include <Matrix.h>
#include <ID.h>
//#include <SimulationInformation.h>

// materials
#include <Steel01.h>
#include <Steel02.h>
#include <Steel4.h>
#include <FatigueMaterial.h>

// integration
#include <LegendreBeamIntegration.h>
#include <LobattoBeamIntegration.h>

// transformation
#include <CorotCrdTransf2d.h>
#include <LinearCrdTransf3d.h>

// fiber
//#include <ElasticSection2d.h>
#include <QuadPatch.h>
#include <CircPatch.h>

// section
#include <UniaxialFiber2d.h>
#include <FiberSection2d.h>

// elements
#include <DispBeamColumn2d.h>
#include <ForceBeamColumn2d.h>
#include <CorotTruss.h>
#include <ElasticBeam2d.h>
#include <ZeroLength.h>

// patterns
#include <LinearSeries.h>
#include <NodalLoad.h>
#include <LoadPattern.h>
//#include <PathSeries.h>
#include <PathTimeSeries.h>
//#include <GroundMotion.h>
//#include <UniformExcitation.h>

// fortran libraries
#ifdef _FORTRAN_LIBS
#include <BandGenLinSOE.h>
#include <BandGenLinLapackSolver.h>
#endif

// solver
#include <Newmark.h>
#include <LoadControl.h>
#include <DisplacementControl.h>
#include <RCM.h>
#include <PlainNumberer.h>
#include <NewtonRaphson.h>
#include <NewtonLineSearch.h>
#include <KrylovNewton.h>
//#include <CTestNormDispIncr.h>
#include <CTestEnergyIncr.h>
#include <PlainHandler.h>
#include <PenaltyConstraintHandler.h>
#include <TransformationConstraintHandler.h>
#include <ProfileSPDLinDirectSolver.h>
#include <ProfileSPDLinSOE.h>
//#include <DirectIntegrationAnalysis.h>
#include <StaticAnalysis.h>
#include <AnalysisModel.h>
#include <SymBandEigenSOE.h>
#include <SymBandEigenSolver.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QHostInfo>

// OpenSees
#include "Domain.h"
#include "StandardStream.h"
StandardStream sserr;
OPS_Stream *opserrPtr = &sserr;
Domain theDomain;

//---------------------------------------------------------------
// Misc. functions
QCheckBox *addCheck(QString text, QString unitText = QObject::tr(""),
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QComboBox *addCombo(QString text, QStringList items, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QDoubleSpinBox *addDoubleSpin(QString text,QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QSpinBox *addSpin(QString text, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);

//---------------------------------------------------------------
// structures
// fiber pointer
struct fiberPointer {
    int fill;
    Fiber **data;
};

//---------------------------------------------------------------
// new window
QWidget *createNewWindow(QString title)
{
    QWidget *window = new QWidget;
    window->show();
    window->setWindowTitle(title);

    return window;
}

// constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    //
    // user settings
    //

    /***************************************
     removing so user remains anonymous
    QSettings settings("SimCenter", "uqFEM");
    QVariant savedValue = settings.value("uuid");

    QUuid uuid;
    if (savedValue.isNull()) {
        uuid = QUuid::createUuid();
        settings.setValue("uuid",uuid);
    } else
        uuid =savedValue.toUuid();
    ******************************************/

    theSteel.a1 = 0.0;
    theSteel.a3 = 0.0;

    //ui->setupUi(this);
    pause = false;
    // constants
    pi = 4*atan(1);

    // create layout and actions
    mainLayout = new QHBoxLayout();
    largeLayout = new QVBoxLayout();
    createActions();

    // Experiment image
    experimentImage = new QLabel();
   // experimentImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    experimentImage->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    
    // create header
    createHeaderBox();

    // load
    loadAISC();

    // create input / output panels
    createInputPanel();
    createOutputPanel();

    largeLayout->addLayout(mainLayout);

    // main widget set to screen size
    QWidget *widget = new QWidget();
    widget->setLayout(largeLayout);
    this->setCentralWidget(widget);

    // create footer
    createFooterBox();

    //
    // adjust size of application window to the available display
    //
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    int height = this->height()<int(0.75*rec.height())?int(0.75*rec.height()):this->height();
    int width  = this->width()<int(0.85*rec.width())?int(0.85*rec.width()):this->width();
    this->resize(width, height);

    // initialize data
    initialize();
    reset();

    inExp->clear();
    inExp->addItem("TCBF3_W8X28.json", ":/ExampleFiles/TCBF3_W8X28.json");
    inExp->addItem("NCBF1_HSS6x6.json", ":/ExampleFiles/NCBF1_HSS6x6.json");

    // access a web page which will increment the usage count for this tool
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://opensees.berkeley.edu/OpenSees/developer/bfm/use.php")));
    //  manager->get(QNetworkRequest(QUrl("https://simcenter.designsafe-ci.org/multiple-degrees-freedom-analytics/")));


    QNetworkRequest request;
    QUrl host("http://www.google-analytics.com/collect");
    request.setUrl(host);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    // setup parameters of request
    QString requestParams;
    QUuid uuid = QUuid::createUuid();
    QString hostname = QHostInfo::localHostName() + "." + QHostInfo::localDomainName();

    requestParams += "v=1"; // version of protocol
    requestParams += "&tid=UA-126287558-1"; // Google Analytics account
    requestParams += "&cid=" + uuid.toString(); // unique user identifier
    requestParams += "&t=event";  // hit type = event others pageview, exception
    requestParams += "&an=BFM";   // app name
    requestParams += "&av=1.0.0"; // app version
    requestParams += "&ec=BFM";   // event category
    requestParams += "&ea=start"; // event action

    // send request via post method
    manager->post(request, requestParams.toStdString().c_str());

}

//---------------------------------------------------------------
MainWindow::~MainWindow()
{
  //    delete ui;
    delete AISCshapes;

    // experiment
    delete time;
    delete expD;
    delete expP;

    // response
    delete Ux;
    delete Uy;
    delete q1;
    delete q2;
    delete q3;
}
//---------------------------------------------------------------
void MainWindow::initialize()
{
    // initialize loading
    numSteps = 2;
    pause = true;

    // experimental data
    expD = new QVector<double>(numSteps,0.);
    expP = new QVector<double>(numSteps,0.);
    time = new QVector<double>(numSteps,0.);
    dt = 0.1;

    // response
    Ux = new Resp();
    Uy = new Resp();
    q1 = new Resp();
    q2 = new Resp();
    q3 = new Resp();

    // slider
    slider->setValue(1);

    // spinBox
    inNe->setValue(10);
    inNIP->setValue(10);
    inNbf->setValue(10);
    inNd->setValue(10);
    inNtw->setValue(10);
    inNtf->setValue(10);

    // initialize QComboBoxes
    inOrient->setCurrentIndex(1);
    inSxn->setCurrentIndex(1);
    inElType->setCurrentIndex(1);
    inElDist->setCurrentIndex(1);
    inIM->setCurrentIndex(1);
    inShape->setCurrentIndex(1);
    inMat->setCurrentIndex(1);
    in_conn1->setCurrentIndex(1);
    in_conn2->setCurrentIndex(1);

    // bools
    matDefault->setChecked(false);
    matAsymm->setChecked(true);
    matFat->setChecked(true);
    connSymm->setChecked(false);
}

// reset
void MainWindow::reset()
{
    stop = false;
    pause = true;

    // remove experiment name
    //inExp->clear();

    // initialize QComboBoxes
    inOrient->setCurrentIndex(0);
    inSxn->setCurrentIndex(0);
    inElType->setCurrentIndex(0);
    inElDist->setCurrentIndex(0);
    inIM->setCurrentIndex(0);
    inShape->setCurrentIndex(0);
    inMat->setCurrentIndex(0);
    in_conn1->setCurrentIndex(0);
    in_conn2->setCurrentIndex(0);

    // spin box
    // reset to trigger valueChanged
    inNe->setValue(2);
    inNIP->setValue(5);
    inNbf->setValue(12);
    inNd->setValue(1);
    inNtw->setValue(2);
    inNtf->setValue(1);

    // reset to trigger valueChanged
    inLwp->setValue(100.0);
    inDelta->setValue(0.2);
    inEs->setValue(29000.0);
    infy->setValue(55.0);

    // fatigue
    inm->setValue(0.458);
    ine0->setValue(0.2);
    inemin->setValue(1.0e16);
    inemax->setValue(1.0e16);

    // rigid end elements
    inRigA_conn1->setValue(10);
    inRigI_conn1->setValue(10);
    //
    inRigA_conn2->setValue(10);
    inRigI_conn2->setValue(10);

    // other params
    angle = pi/4;

    // check box
    matDefault->setChecked(true);
    matAsymm->setChecked(false);
    matFat->setChecked(false);
    connSymm->setChecked(true);

    // initialize experiment
   //Experiment *exp = new Experiment();
   //setExp(exp);

    // Load default experiments
    // inExp->setCurrentText("TCBF3_W8X28.json");
    if (inExp->count() != 0) {
        inExp->setCurrentIndex(0);
        this->loadExperimentalFile(inExp->itemData(0).toString());
        hPlot->plotModel();
    }
}

bool MainWindow::saveFile(const QString &fileName)
{
    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }
    currentFile = fileName;


    //
    // create a json object, fill it in & then use a QJsonDocument
    // to write the contents of the object to the file in JSON format
    //

    QJsonObject json;
    QJsonObject element;
    QJsonObject section;
    QJsonObject material;
    QJsonObject connection;

    // Add element data
    element.insert(QStringLiteral("elementModel"), inElType->currentText());
    element.insert(QStringLiteral("workPointLength"), inLwp->value());
    element.insert(QStringLiteral("braceLength"), inL->value());
    element.insert(QStringLiteral("numSubElements"), inNe->value());
    element.insert(QStringLiteral("numIntegrationPoints"), inNIP->value());
    element.insert(QStringLiteral("camber"), inDelta->value());
    element.insert(QStringLiteral("subElDistribution"), inElDist->currentText());
    element.insert(QStringLiteral("integrationMethod"), inIM->currentText());
    element.insert(QStringLiteral("camberShape"), inShape->currentText());
    json.insert(QStringLiteral("element"), element);

    // Add section data
    section.insert(QStringLiteral("sectionType"), inSxn->currentText());
    section.insert(QStringLiteral("orientation"), inOrient->currentText());
    section.insert(QStringLiteral("nbf"), inNbf->value());
    section.insert(QStringLiteral("ntf"), inNtf->value());
    section.insert(QStringLiteral("nd"), inNd->value());
    section.insert(QStringLiteral("ntw"), inNtw->value());
    json.insert(QStringLiteral("section"), section);

    // Add material data
    material.insert(QStringLiteral("includeFatigue"), matFat->isChecked());
    material.insert(QStringLiteral("useDefaults"), matDefault->isChecked());
    material.insert(QStringLiteral("E"), inEs->value());
    material.insert(QStringLiteral("fy"), infy->value());
    // Add fatigue settings
    QJsonObject fatigue;
    fatigue.insert(QStringLiteral("m"), inm->value());
    fatigue.insert(QStringLiteral("e0"), ine0->value());
    fatigue.insert(QStringLiteral("emin"), inemin->value());
    fatigue.insert(QStringLiteral("emax"), inemax->value());
    material.insert("fatigue", fatigue);

    // Add material model settings
    QJsonObject materialModel;
    materialModel.insert("model", inMat->currentText());
    
    switch (inMat->currentIndex()) {
       // Uniaxial bi-linear material model
       case 0: {
	 QJsonObject kinematicHardening;
	 QJsonObject isotropicHardening;
	 // Kinematic hardening settings
	 kinematicHardening.insert(QStringLiteral("b"), inb->value());
	 // Isotropic hardening settings
	 isotropicHardening.insert(QStringLiteral("a1"), ina1->value());
	 isotropicHardening.insert(QStringLiteral("a2"), ina2->value());
	 isotropicHardening.insert(QStringLiteral("a3"), ina3->value());
	 isotropicHardening.insert(QStringLiteral("a4"), ina4->value());
	 // Add hardening settings to material model
	 materialModel.insert(QStringLiteral("kinematicHardening"), kinematicHardening);
	 materialModel.insert(QStringLiteral("isotropicHardening"), isotropicHardening); 
	 break;
       }

       // Uniaxial Giuffre-Menegotto-Pinto model
       case 1: {
	 QJsonObject kinematicHardening;
	 QJsonObject isotropicHardening;
	 QJsonObject hardeningTrans;
	 // Kinematic hardening settings
	 kinematicHardening.insert(QStringLiteral("b"), inb->value());
	 // Isotropic hardening settings
	 isotropicHardening.insert(QStringLiteral("a1"), ina1->value());
	 isotropicHardening.insert(QStringLiteral("a2"), ina2->value());
	 isotropicHardening.insert(QStringLiteral("a3"), ina3->value());
	 isotropicHardening.insert(QStringLiteral("a4"), ina4->value());
	 // Add elast to hardening transitions
	 hardeningTrans.insert(QStringLiteral("R0"), inR0->value());
	 hardeningTrans.insert(QStringLiteral("r1"), inR1->value());
	 hardeningTrans.insert(QStringLiteral("r2"), inR2->value());	 
	 // Add hardening settings to material model
	 materialModel.insert(QStringLiteral("kinematicHardening"), kinematicHardening);
	 materialModel.insert(QStringLiteral("isotropicHardening"), isotropicHardening);
	 materialModel.insert(QStringLiteral("hardeningTransitions"), hardeningTrans);
	 break;
       }
	 
       // Uniaxial asymmetric Giuffre-Menegotto-Pinto model
       case 2: {
	 QJsonObject kinematicHardening;
	 QJsonObject kinematicHardeningTension;
	 QJsonObject kinematicHardeningComp;
	 QJsonObject isotropicHardening;
	 QJsonObject isotropicHardeningTension;
	 QJsonObject isotropicHardeningComp;	 
	 // Kinematic hardening settings
	 kinematicHardeningTension.insert(QStringLiteral("b"), inbk->value());
	 kinematicHardeningTension.insert(QStringLiteral("R0"), inR0k->value());
	 kinematicHardeningTension.insert(QStringLiteral("r1"), inr1->value());
	 kinematicHardeningTension.insert(QStringLiteral("r2"), inr2->value());
	 kinematicHardeningComp.insert(QStringLiteral("b"), inbkc->value());
	 kinematicHardeningComp.insert(QStringLiteral("R0"), inR0kc->value());
	 kinematicHardeningComp.insert(QStringLiteral("r1"), inr1c->value());
	 kinematicHardeningComp.insert(QStringLiteral("r2"), inr2c->value());
	 kinematicHardening.insert(QStringLiteral("tension"), kinematicHardeningTension);
	 kinematicHardening.insert(QStringLiteral("compression"), kinematicHardeningComp);
	 // Isotropic hardening settings
	 isotropicHardeningTension.insert(QStringLiteral("b"), inbi->value());
	 isotropicHardeningTension.insert(QStringLiteral("rho"), inrhoi->value());
	 isotropicHardeningTension.insert(QStringLiteral("bl"), inbl->value());
	 isotropicHardeningTension.insert(QStringLiteral("Ri"), inRi->value());
	 isotropicHardeningTension.insert(QStringLiteral("lyp"), inlyp->value());
	 isotropicHardeningComp.insert(QStringLiteral("b"), inbic->value());
	 isotropicHardeningComp.insert(QStringLiteral("rho"), inrhoic->value());
	 isotropicHardeningComp.insert(QStringLiteral("bl"), inblc->value());
	 isotropicHardeningComp.insert(QStringLiteral("Ri"), inRic->value());
	 isotropicHardening.insert(QStringLiteral("tension"), isotropicHardeningTension);
	 isotropicHardening.insert(QStringLiteral("compression"), isotropicHardeningComp);
	 // Add hardering settings to material model
	 materialModel.insert(QStringLiteral("asymmetric"), matAsymm->isChecked());
	 materialModel.insert(QStringLiteral("kinematicHardening"), kinematicHardening);
	 materialModel.insert(QStringLiteral("isotropicHardening"), isotropicHardening);
	 break;
       }
    }

    // Add material model to material
    material.insert(QStringLiteral("materialModel"), materialModel);
    json.insert(QStringLiteral("material"), material);

    // Add connection data
    QJsonObject connection_1;
    QJsonObject connection_2;
    connection_1.insert(QStringLiteral("model"), in_conn1->currentText());
    connection_1.insert(QStringLiteral("gussetLength"), inl_conn1->value());
    connection_1.insert(QStringLiteral("A"), inRigA_conn1->value());
    connection_1.insert(QStringLiteral("I"), inRigI_conn1->value());
    connection_2.insert(QStringLiteral("model"), in_conn2->currentText());
    connection_2.insert(QStringLiteral("gussetLength"), inl_conn2->value());
    connection_2.insert(QStringLiteral("A"), inRigA_conn2->value());
    connection_2.insert(QStringLiteral("I"), inRigI_conn2->value());

    connection.insert(QStringLiteral("connection1"), connection_1);
    connection.insert(QStringLiteral("connection2"), connection_2);
    connection.insert(QStringLiteral("symmetricConnections"), connSymm->isChecked());
    
    json.insert(QStringLiteral("connections"), connection);
      
    // Add test information
    QJsonObject test;
    QJsonArray axialDeformation;
    QJsonArray axialForce;
    QJsonArray timeSteps;
    
    for (int i = 0; i < time->size(); ++i) {
      axialDeformation.push_back((*expD)[i]);
      axialForce.push_back((*expP)[i]);
      timeSteps.push_back((*time)[i]);
    }
    
    test.insert(QStringLiteral("type"), experimentType);
    test.insert(QStringLiteral("axialDef"), axialDeformation);
    test.insert(QStringLiteral("axialForce"), axialForce);
    test.insert(QStringLiteral("timeSteps"), timeSteps);
    json.insert(QStringLiteral("test"), test);   

    QJsonDocument doc(json);
    file.write(doc.toJson());

    // close file
    file.close();

    return true;
}

// read file
void MainWindow::loadFile(const QString &fileName)
{
    // open files
    QFile mFile(fileName);

    // open warning
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), mFile.errorString()));
        return;
    }

    // place file contents into json object
    QString mText = mFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(mText.toUtf8());
    if (doc.isNull() || doc.isEmpty()) {
        QMessageBox::warning(this, tr("Application"),
                tr("Error loading file: not a JSON file or is empty."));
        return;
    }
    QJsonObject jsonObject = doc.object();
    QJsonValue json;    

    // Read input JSON for saved analysis
    if (jsonObject["brace"].isNull() || jsonObject["brace"].isUndefined()) {
      // Load element data
      json = jsonObject["element"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Element data not specified.");
      else {
	QJsonObject theData = json.toObject();
	inElType->setCurrentText(theData["elementModel"].toString());
	inLwp->setValue(theData["workPointLength"].toDouble());
	inL->setValue(theData["braceLength"].toDouble());
	inNe->setValue(theData["numSubElements"].toInt());
	inNIP->setValue(theData["numIntegrationPoints"].toInt());
	inDelta->setValue(theData["camber"].toDouble());
	inElDist->setCurrentText(theData["subElDistribution"].toString());
	inIM->setCurrentText(theData["integrationMethod"].toString());
	inShape->setCurrentText(theData["camberShape"].toString());	
      }

      // Load section data
      json = jsonObject["section"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Section data not specified.");
      else {
	QJsonObject theData = json.toObject();
	inSxn->setCurrentText(theData["sectionType"].toString());
	inOrient->setCurrentText(theData["orientation"].toString());
	inNbf->setValue(theData["nbf"].toInt());
	inNtf->setValue(theData["ntf"].toInt());
	inNd->setValue(theData["nd"].toInt());
	inNtw->setValue(theData["ntw"].toInt());
      }

      // Load material data
      json = jsonObject["material"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Material data not specified.");
      else {
	QJsonObject theData = json.toObject();
	QJsonObject theOtherData;
	QJsonObject theOtherOtherData;
	matFat->setChecked(theData["includeFatigue"].toBool());
	matDefault->setChecked(theData["useDefaults"].toBool());
	inEs->setValue(theData["E"].toDouble());
	infy->setValue(theData["fy"].toDouble());
	// Set fatigue data
	if (theData["fatigue"].isNull() || theData["fatigue"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Fatigue data not specified.");	  
	} else {
	  theOtherData = theData["fatigue"].toObject();
	  inm->setValue(theOtherData["m"].toDouble());
	  ine0->setValue(theOtherData["e0"].toDouble());
	  inemin->setValue(theOtherData["emin"].toDouble());
	  inemax->setValue(theOtherData["emax"].toDouble());	  
	}
	// Set material model
	if (theData["materialModel"].isNull() || theData["materialModel"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Material model data not specified.");	  	  
	} else {
	  theOtherData = theData["materialModel"].toObject();
	  inMat->setCurrentText(theOtherData["model"].toString());	  
	}

	switch (inMat->currentIndex()) {
	  // Uniaxial bi-linear material model	  
	  case 0: {
	    // Kinematic hardening
	    if (theOtherData["kinematicHardening"].isNull() || theOtherData["kinematicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Kinematic hardening data not specified.");	  	  	      
	    } else {
	      theOtherOtherData = theOtherData["kinematicHardening"].toObject();
	      inb->setValue(theOtherOtherData["b"].toDouble());	      
	    }
	    // Isotropic hardening
	    if (theOtherData["isotropicHardening"].isNull() || theOtherData["isotropicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Isotropic hardening data not specified.");	      
	    } else {
	      theOtherOtherData = theOtherData["isotropicHardening"].toObject();
	      ina1->setValue(theOtherOtherData["a1"].toDouble());
	      ina2->setValue(theOtherOtherData["a2"].toDouble());
	      ina3->setValue(theOtherOtherData["a3"].toDouble());
	      ina4->setValue(theOtherOtherData["a4"].toDouble());	      
	    }
	    break;
	  }

	  // Uniaxial Giuffre-Menegotto-Pinto model	    
	  case 1: {
	    // Kinematic hardening
	    if (theOtherData["kinematicHardening"].isNull() || theOtherData["kinematicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Kinematic hardening data not specified.");	  	  	      
	    } else {
	      theOtherOtherData = theOtherData["kinematicHardening"].toObject();
	      inb->setValue(theOtherOtherData["b"].toDouble());	      
	    }
	    // Isotropic hardening
	    if (theOtherData["isotropicHardening"].isNull() || theOtherData["isotropicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Isotropic hardening data not specified.");	      
	    } else {
	      theOtherOtherData = theOtherData["isotropicHardening"].toObject();
	      ina1->setValue(theOtherOtherData["a1"].toDouble());
	      ina2->setValue(theOtherOtherData["a2"].toDouble());
	      ina3->setValue(theOtherOtherData["a3"].toDouble());
	      ina4->setValue(theOtherOtherData["a4"].toDouble());	      
	    }
	    // Hardening transitions
	    if (theOtherData["hardeningTransitions"].isNull() || theOtherData["hardeningTransistions"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Hardening transitions data not specified.");	      	      
	    } else {
	      theOtherOtherData = theOtherData["hardeningTransitions"].toObject();
	      inR0->setValue(theOtherOtherData["R0"].toDouble());
	      inR1->setValue(theOtherOtherData["r1"].toDouble());
	      inR2->setValue(theOtherOtherData["r2"].toDouble());	      
	    }
	    break;
	  }

          // Uniaxial asymmetric Giuffre-Menegotto-Pinto model	    
	  case 2: {
	    // Kinematic hardening
	    if (theOtherData["kinematicHardening"].isNull() || theOtherData["kinematicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Kinematic hardening data not specified.");	  	  	      
	    } else {
	      // Tension
	      theOtherOtherData = theOtherData["kinematicHardening"].toObject();
	      if (theOtherOtherData["tension"].isNull() || theOtherOtherData["tension"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Kinematic hardening tension data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["tension"].toObject();
		inbk->setValue(lottaData["b"].toDouble());
		inR0k->setValue(lottaData["R0"].toDouble());
		inr1->setValue(lottaData["r1"].toDouble());
		inr2->setValue(lottaData["r2"].toDouble());
	      }
	      // Compression
	      if (theOtherOtherData["compression"].isNull() || theOtherOtherData["compression"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Kinematic hardening compression data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["compression"].toObject();
		inbkc->setValue(lottaData["b"].toDouble());
		inR0kc->setValue(lottaData["R0"].toDouble());
		inr1c->setValue(lottaData["r1"].toDouble());
		inr2c->setValue(lottaData["r2"].toDouble());		
	      } 
	    }
	    // Isotropic hardening
	    if (theOtherData["isotropicHardening"].isNull() || theOtherData["isotropicHardering"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Isotropic hardening data not specified.");	      
	    } else {
	      // Tension
	      theOtherOtherData = theOtherData["isotropicHardening"].toObject();
	      if (theOtherOtherData["tension"].isNull() || theOtherOtherData["tension"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Isotropic hardening tension data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["tension"].toObject();
		inbi->setValue(lottaData["b"].toDouble());
		inrhoi->setValue(lottaData["rho"].toDouble());
		inbl->setValue(lottaData["bl"].toDouble());
		inRi->setValue(lottaData["Ri"].toDouble());
		inlyp->setValue(lottaData["lyp"].toDouble());
	      }
	      // Compression
	      if (theOtherOtherData["compression"].isNull() || theOtherOtherData["compression"].isUndefined()) {
		QMessageBox::warning(this, "Warning","Isotropic hardening compression data not specified.");
	      } else {
		QJsonObject lottaData = theOtherOtherData["compression"].toObject();		
		inbic->setValue(lottaData["b"].toDouble());
		inrhoic->setValue(lottaData["rho"].toDouble());
		inblc->setValue(lottaData["bl"].toDouble());
		inRic->setValue(lottaData["Ri"].toDouble());
	      }
	    }
	    matAsymm->setChecked(theOtherData["asymmetric"].toBool());
	    break;
	  }

	  default: {
	    QMessageBox::warning(this, "Warning","Material model specified does not exist or specified incorrectly.");
	    break;
	  }
	}
      }

      // Load connection data
      json = jsonObject["connections"];
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Connection data not specified.");
      else {
	// Connection 1
	QJsonObject theData = json.toObject();
	QJsonObject connection;
	if (theData["connection1"].isNull() || theData["connection1"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Connection 1 data not specified.");	  
	} else {
	  connection = theData["connection1"].toObject();
	  in_conn1->setCurrentText(connection["model"].toString());
	  inl_conn1->setValue(connection["gussetLength"].toDouble());
	  inRigA_conn1->setValue(connection["A"].toDouble());
	  inRigI_conn1->setValue(connection["I"].toDouble());
	}
	// Connection 2
	if (theData["connection2"].isNull() || theData["connection2"].isUndefined()) {
	  QMessageBox::warning(this, "Warning","Connection 2 data not specified.");	  
	} else {
	  connection = theData["connection2"].toObject();
	  in_conn2->setCurrentText(connection["model"].toString());
	  inl_conn2->setValue(connection["gussetLength"].toDouble());
	  inRigA_conn2->setValue(connection["A"].toDouble());
	  inRigI_conn2->setValue(connection["I"].toDouble());	  
	}

	connSymm->setChecked(theData["symmetricConnections"].toBool());
      }

    } else {
        qDebug() << "HERE IN LOAD";

      // read brace
      json = jsonObject["brace"];

      // load brace data
      if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Brace data not specified.");

      else {
        QJsonObject theData = json.toObject();

        // brace section
        if (theData["sxn"].isNull() || theData["sxn"].isUndefined())
	  QMessageBox::warning(this, "Warning","Section not specified.");

        else {
	  QString text = theData["sxn"].toString();
	  int index = inSxn->findText(text);

	  if (index != -1) {
	    inSxn->setCurrentIndex(index);

	  } else
	    QMessageBox::warning(this, "Warning","Loaded section not in current AISC Shape Database.");
        }

        // orient
        if (theData["orient"].isNull() || theData["orient"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace: Orientation not specified.");

        else {
	  QString text = theData["orient"].toString();
	  int index = inOrient->findText(text);

	  if (index != -1) {
	    inOrient->setCurrentIndex(index);

	  } else
	    QMessageBox::warning(this, "Warning","Orientation not defined.");
        }

        // brace length
        if ((theData["width"].isNull())     || (theData["height"].isNull())
	    || theData["width"].isUndefined() || theData["height"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace length not specified.");

        else {
	  braceWidth = theData["width"].toDouble();
	  braceHeight = theData["height"].toDouble();

	  Lwp = sqrt(pow(braceWidth,2)+pow(braceHeight,2));
	  inLwp->setValue(Lwp);
	  angle = atan(braceHeight/braceWidth);
        }

        // fy
        if (theData["fy"].isNull() || theData["fy"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace: fy not specified.");

        else {
	  theSteel.fy=theData["fy"].toDouble();
	  infy->setValue(theSteel.fy);
        }

        // Es
        if (theData["E"].isNull() || theData["E"].isUndefined())
	  QMessageBox::warning(this, "Warning","Brace: Es not specified.");

        else {
	  theSteel.Es=theData["E"].toDouble();
	  inEs->setValue(theSteel.Es);
        }
      }

      // read connection-1
      json = jsonObject["connection-1"];

      // load brace data
      if (json.isNull() || json.isUndefined()) {
        QMessageBox::warning(this, "Warning","Connection-1 data not specified. \nConnection set to 5% workpoint length.");
        inl_conn1->setValue(0.05*Lwp);

      } else {
        QJsonObject theData = json.toObject();

	conn1.fy = theData["fy"].toDouble();
	conn1.Es = theData["E"].toDouble();
	conn1.tg = theData["tg"].toDouble();
	conn1.H = theData["H"].toDouble();
	conn1.W = theData["W"].toDouble();
	conn1.lb = theData["lb"].toDouble();
	conn1.lc = theData["lc"].toDouble();
	conn1.lbr = theData["lbr"].toDouble();
	conn1.eb = theData["eb"].toDouble();
	conn1.ec = theData["ec"].toDouble();
	
        // geometry
        if (theData["H"].isNull() || theData["H"].isUndefined()
	    || theData["W"].isNull() || theData["W"].isUndefined()
	    || theData["lb"].isNull() || theData["lb"].isUndefined()
	    || theData["lc"].isNull() || theData["lc"].isUndefined()
	    || theData["lbr"].isNull() || theData["lbr"].isUndefined()
	    || theData["eb"].isNull() || theData["eb"].isUndefined()
	    || theData["ec"].isNull() || theData["ec"].isUndefined())
	  {
            if (theData["L"].isNull() || theData["L"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Connection-1: not enough geometric information. \nConnection set to 5% workpoint length.");
	      inl_conn1->setValue(0.05*Lwp);

            } else {
	      double L2=theData["L"].toDouble();
	      inl_conn1->setValue(L2);
            }
	  }

        else {
	  double H=theData["H"].toDouble();
	  double W=theData["W"].toDouble();
	  double lb=theData["lb"].toDouble();
	  double lc=theData["lc"].toDouble();
	  double lbr=theData["lbr"].toDouble();
	  double eb=theData["eb"].toDouble();
	  double ec=theData["ec"].toDouble();

	  // estimate the whitmore width
	  double c = 0.5*sqrt(pow(W - lb,2)+pow(H - lc,2));
	  double lw = 2*lbr*tan(30*pi/180) + 2*c;

	  // calculate connection length
	  double w = lc*tan(angle)+c/sin(angle);
	  double L2;
	  if (W <= w)
	    L2 = W/cos(angle) - c*tan(angle) - lbr + ec/(2*cos(angle));
	  else
	    L2 = w/cos(angle) - c*tan(angle) - lbr + eb/(2*sin(angle));

	  inl_conn1->setValue(L2);
        }
      }

      // read connection-2
      json = jsonObject["connection-2"];

      // load brace data
      if (json.isNull() || json.isUndefined()) {
        QMessageBox::warning(this, "Warning","Connection-1 data not specified. \nConnection set to 5% workpoint length.");
        inl_conn1->setValue(0.05*Lwp);

      } else {
        QJsonObject theData = json.toObject();

	conn2.fy = theData["fy"].toDouble();
	conn2.Es = theData["E"].toDouble();
	conn2.tg = theData["tg"].toDouble();
	conn2.H = theData["H"].toDouble();
	conn2.W = theData["W"].toDouble();
	conn2.lb = theData["lb"].toDouble();
	conn2.lc = theData["lc"].toDouble();
	conn2.lbr = theData["lbr"].toDouble();
	conn2.eb = theData["eb"].toDouble();
	conn2.ec = theData["ec"].toDouble();

	// geometry
        if (theData["H"].isNull() || theData["H"].isUndefined()
	    || theData["W"].isNull() || theData["W"].isUndefined()
	    || theData["lb"].isNull() || theData["lb"].isUndefined()
	    || theData["lc"].isNull() || theData["lc"].isUndefined()
	    || theData["lbr"].isNull() || theData["lbr"].isUndefined()
	    || theData["eb"].isNull() || theData["eb"].isUndefined()
	    || theData["ec"].isNull() || theData["ec"].isUndefined())
	  {
            if (theData["L"].isNull() || theData["L"].isUndefined()) {
	      QMessageBox::warning(this, "Warning","Connection-1: not enough geometric information. \nConnection set to 5% workpoint length.");
	      inl_conn2->setValue(0.05*Lwp);

            } else {
	      double L2=theData["L"].toDouble();
	      inl_conn2->setValue(L2);
            }
	  }

        else {
	  double H=theData["H"].toDouble();
	  double W=theData["W"].toDouble();
	  double lb=theData["lb"].toDouble();
	  double lc=theData["lc"].toDouble();
	  double lbr=theData["lbr"].toDouble();
	  double eb=theData["eb"].toDouble();
	  double ec=theData["ec"].toDouble();

	  // estimate the whitmore width
	  double c = 0.5*sqrt(pow(W - lb,2)+pow(H - lc,2));
	  double lw = 2*lbr*tan(30*pi/180) + 2*c;

	  // calculate connection length
	  double w = lc*tan(angle)+c/sin(angle);
	  double L2;
	  if (W <= w)
	    L2 = W/cos(angle) - c*tan(angle) - lbr + ec/(2*cos(angle));
	  else
	    L2 = w/cos(angle) - c*tan(angle) - lbr + eb/(2*sin(angle));

	  inl_conn2->setValue(L2);
        }
      }

      // re-set symm connections
      connSymm->setCheckState(Qt::Unchecked);      
    }

    // read experiment loading
    json = jsonObject["test"];

    Experiment *exp = new Experiment();
    int ok = exp->inputFromJSON(json);

    if (ok == -1)
        QMessageBox::warning(this, "Warning","Experiment loading not specified.");
    else if (ok == -2)
        QMessageBox::warning(this, "Warning","Experiment history: axial deformation not specified.");
    else if (ok == -3)
        QMessageBox::warning(this, "Warning","Experiment history: axial force not specified.");
    else if (ok == -4)
        QMessageBox::warning(this, "Warning","Loaded axial force and deformation history are not the same length. Histories are truncated accordingly.");
    else if (ok == -5) {
      QMessageBox::warning(this, "Warning", "Experiment test type not specified."); 
    }

    inExp->setCurrentIndex(0);

    /*
    setExp(exp);

    // Set test type
    experimentType = exp->getTestType();
    
    // name experiment
    QString name = fileName.section("/", -1, -1);

    // set as current    
    if (inExp->findText(name) == -1) {
      inExp->addItem(name, fileName);
    }
    inExp->setCurrentIndex(inExp->findText(name));
*/
    // close file
    mFile.close();
}




// read experimental file
void MainWindow::loadExperimentalFile(const QString &fileName)
{
    // open files
    QFile mFile(fileName);

    // open warning
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), mFile.errorString()));
        return;
    }

    // place file contents into json object
    QString mText = mFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(mText.toUtf8());
    if (doc.isNull() || doc.isEmpty()) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Error loading file: not a JSON file or is empty."));
        return;
    }
    QJsonObject jsonObject = doc.object();
    QJsonValue json;

    // read brace
    json = jsonObject["brace"];

    // load brace data
    if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Brace data not specified.");

    else {
        QJsonObject theData = json.toObject();

        // brace section
        if (theData["sxn"].isNull() || theData["sxn"].isUndefined())
            QMessageBox::warning(this, "Warning","Section not specified.");

        else {
            QString text = theData["sxn"].toString();
            int index = inSxn->findText(text);

            if (index != -1) {
                inSxn->setCurrentIndex(index);

            } else
                QMessageBox::warning(this, "Warning","Loaded section not in current AISC Shape Database.");
        }

        // orient
        if (theData["orient"].isNull() || theData["orient"].isUndefined())
            QMessageBox::warning(this, "Warning","Brace: Orientation not specified.");

        else {
            QString text = theData["orient"].toString();
            int index = inOrient->findText(text);

            if (index != -1) {
                inOrient->setCurrentIndex(index);

            } else
                QMessageBox::warning(this, "Warning","Orientation not defined.");
        }

        // brace length
        if ((theData["width"].isNull())     || (theData["height"].isNull())
                || theData["width"].isUndefined() || theData["height"].isUndefined())
            QMessageBox::warning(this, "Warning","Brace length not specified.");

        else {
            braceWidth = theData["width"].toDouble();
            braceHeight = theData["height"].toDouble();

            Lwp = sqrt(pow(braceWidth,2)+pow(braceHeight,2));
            inLwp->setValue(Lwp);
            angle = atan(braceHeight/braceWidth);
        }

        // fy
        if (theData["fy"].isNull() || theData["fy"].isUndefined())
            QMessageBox::warning(this, "Warning","Brace: fy not specified.");

        else {
            theSteel.fy=theData["fy"].toDouble();
            infy->setValue(theSteel.fy);
        }

        // Es
        if (theData["E"].isNull() || theData["E"].isUndefined())
            QMessageBox::warning(this, "Warning","Brace: Es not specified.");

        else {
            theSteel.Es=theData["E"].toDouble();
            inEs->setValue(theSteel.Es);
        }
    }

    // read connection-1
    json = jsonObject["connection-1"];

    // load brace data
    if (json.isNull() || json.isUndefined()) {
        QMessageBox::warning(this, "Warning","Connection-1 data not specified. \nConnection set to 5% workpoint length.");
        inl_conn1->setValue(0.05*Lwp);

    } else {
        QJsonObject theData = json.toObject();

        conn1.fy = theData["fy"].toDouble();
        conn1.Es = theData["E"].toDouble();
        conn1.tg = theData["tg"].toDouble();
        conn1.H = theData["H"].toDouble();
        conn1.W = theData["W"].toDouble();
        conn1.lb = theData["lb"].toDouble();
        conn1.lc = theData["lc"].toDouble();
        conn1.lbr = theData["lbr"].toDouble();
        conn1.eb = theData["eb"].toDouble();
        conn1.ec = theData["ec"].toDouble();

        // geometry
        if (theData["H"].isNull() || theData["H"].isUndefined()
                || theData["W"].isNull() || theData["W"].isUndefined()
                || theData["lb"].isNull() || theData["lb"].isUndefined()
                || theData["lc"].isNull() || theData["lc"].isUndefined()
                || theData["lbr"].isNull() || theData["lbr"].isUndefined()
                || theData["eb"].isNull() || theData["eb"].isUndefined()
                || theData["ec"].isNull() || theData["ec"].isUndefined())
        {
            if (theData["L"].isNull() || theData["L"].isUndefined()) {
                QMessageBox::warning(this, "Warning","Connection-1: not enough geometric information. \nConnection set to 5% workpoint length.");
                inl_conn1->setValue(0.05*Lwp);

            } else {
                double L2=theData["L"].toDouble();
                inl_conn1->setValue(L2);
            }
        }

        else {
            double H=theData["H"].toDouble();
            double W=theData["W"].toDouble();
            double lb=theData["lb"].toDouble();
            double lc=theData["lc"].toDouble();
            double lbr=theData["lbr"].toDouble();
            double eb=theData["eb"].toDouble();
            double ec=theData["ec"].toDouble();

            // estimate the whitmore width
            double c = 0.5*sqrt(pow(W - lb,2)+pow(H - lc,2));
            double lw = 2*lbr*tan(30*pi/180) + 2*c;

            // calculate connection length
            double w = lc*tan(angle)+c/sin(angle);
            double L2;
            if (W <= w)
                L2 = W/cos(angle) - c*tan(angle) - lbr + ec/(2*cos(angle));
            else
                L2 = w/cos(angle) - c*tan(angle) - lbr + eb/(2*sin(angle));

            inl_conn1->setValue(L2);
        }
    }

    // read connection-2
    json = jsonObject["connection-2"];

    // load brace data
    if (json.isNull() || json.isUndefined()) {
        QMessageBox::warning(this, "Warning","Connection-1 data not specified. \nConnection set to 5% workpoint length.");
        inl_conn1->setValue(0.05*Lwp);

    } else {
        QJsonObject theData = json.toObject();

        conn2.fy = theData["fy"].toDouble();
        conn2.Es = theData["E"].toDouble();
        conn2.tg = theData["tg"].toDouble();
        conn2.H = theData["H"].toDouble();
        conn2.W = theData["W"].toDouble();
        conn2.lb = theData["lb"].toDouble();
        conn2.lc = theData["lc"].toDouble();
        conn2.lbr = theData["lbr"].toDouble();
        conn2.eb = theData["eb"].toDouble();
        conn2.ec = theData["ec"].toDouble();

        // geometry
        if (theData["H"].isNull() || theData["H"].isUndefined()
                || theData["W"].isNull() || theData["W"].isUndefined()
                || theData["lb"].isNull() || theData["lb"].isUndefined()
                || theData["lc"].isNull() || theData["lc"].isUndefined()
                || theData["lbr"].isNull() || theData["lbr"].isUndefined()
                || theData["eb"].isNull() || theData["eb"].isUndefined()
                || theData["ec"].isNull() || theData["ec"].isUndefined())
        {
            if (theData["L"].isNull() || theData["L"].isUndefined()) {
                QMessageBox::warning(this, "Warning","Connection-1: not enough geometric information. \nConnection set to 5% workpoint length.");
                inl_conn2->setValue(0.05*Lwp);

            } else {
                double L2=theData["L"].toDouble();
                inl_conn2->setValue(L2);
            }
        }

        else {
            double H=theData["H"].toDouble();
            double W=theData["W"].toDouble();
            double lb=theData["lb"].toDouble();
            double lc=theData["lc"].toDouble();
            double lbr=theData["lbr"].toDouble();
            double eb=theData["eb"].toDouble();
            double ec=theData["ec"].toDouble();

            // estimate the whitmore width
            double c = 0.5*sqrt(pow(W - lb,2)+pow(H - lc,2));
            double lw = 2*lbr*tan(30*pi/180) + 2*c;

            // calculate connection length
            double w = lc*tan(angle)+c/sin(angle);
            double L2;
            if (W <= w)
                L2 = W/cos(angle) - c*tan(angle) - lbr + ec/(2*cos(angle));
            else
                L2 = w/cos(angle) - c*tan(angle) - lbr + eb/(2*sin(angle));

            inl_conn2->setValue(L2);
        }
    }

    // re-set symm connections
    connSymm->setCheckState(Qt::Unchecked);


    // read experiment loading
    json = jsonObject["test"];

    Experiment *exp = new Experiment();
    int ok = exp->inputFromJSON(json);

    if (ok == -1)
        QMessageBox::warning(this, "Warning","Experiment loading not specified.");
    else if (ok == -2)
        QMessageBox::warning(this, "Warning","Experiment history: axial deformation not specified.");
    else if (ok == -3)
        QMessageBox::warning(this, "Warning","Experiment history: axial force not specified.");
    else if (ok == -4)
        QMessageBox::warning(this, "Warning","Loaded axial force and deformation history are not the same length. Histories are truncated accordingly.");
    else if (ok == -5) {
        QMessageBox::warning(this, "Warning", "Experiment test type not specified.");
    }

    setExp(exp);

    // Set test type
    experimentType = exp->getTestType();

    // name experiment
    QString name = fileName.section("/", -1, -1);

    // set as current
    if (inExp->findText(name) == -1) {
        inExp->addItem(name, fileName);
    }
    inExp->setCurrentIndex(inExp->findText(name));
    // Add experiment image
    QString imageName = ":MyResources/" + name.section(".", -2, -2) + ".png";
    QPixmap pixmap(imageName);
    pixmap = pixmap.scaledToHeight(300);
    experimentImage->setPixmap(pixmap);

    // close file
    mFile.close();
}





// load experiment
void MainWindow::addExp_clicked()
{
    // call load file function
    QString Filename = QFileDialog::getOpenFileName(this);
    if (!Filename.isEmpty())
        loadExperimentalFile(Filename);
}
//---------------------------------------------------------------
// load AISC Shape Database
void MainWindow::loadAISC()
{
    // resource name
    QString Filename = ":/MyResources/aisc-shapes-database-v15.0.csv";

    // open file
    QFile mFile(Filename);
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(Filename), mFile.errorString()));
        return;
    }

    // creat csv model
    //AISCshapes = new QTableWidget(this);
    AISCshapes = new QStandardItemModel(this);

    // read file
    QTextStream mText(&mFile);
    while (!mText.atEnd())
    {
        // read line-by-line
        QString line = mText.readLine();

        // add to list
        QList<QStandardItem *> newItem;
        for (QString item : line.split(","))
        {
            newItem.append(new QStandardItem(item));
        }

        // add to model
        AISCshapes->insertRow(AISCshapes->rowCount(),newItem);
    }

    // properties
    QList<QStandardItem *> AISCprop = AISCshapes->takeRow(0);
    for (int i = 1; i < AISCprop.size(); i++)
    {
        propList.append(AISCprop.at(i)->text());
    }

    // sections
    QList<QStandardItem *> AISCsxn = AISCshapes->takeColumn(0);
    for (int i = 0; i < AISCsxn.size(); i++)
    {
        sxnList.append(AISCsxn.at(i)->text());
    }

    // headers
    AISCshapes->setHorizontalHeaderLabels(propList);
    AISCshapes->setVerticalHeaderLabels(sxnList);

    // close file
    mFile.close();
}

// AISC button
void MainWindow::addAISC_clicked()
{
    QTableView *table = new QTableView;
    table->setModel(AISCshapes);
    table->setWindowTitle("AISC Shape Database");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->selectionModel()->selectedRows();
    table->show();
    int index = inSxn->findText(sxn);
    if (index != -1) {
        table->selectRow(index);
    }

    //
    // adjust size to the available display
    //
    QRect rec = QGuiApplication::primaryScreen()->geometry();
    table->resize(int(0.65*rec.width()), int(0.65*rec.height()));
    qDebug() << .65*rec.width();

    // connect signals / slots
    connect(table->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(theAISC_sectionClicked(int)));
}

// select section in AISC table
void MainWindow::theAISC_sectionClicked(int row)
{
    inSxn->setCurrentIndex(row);
}
//---------------------------------------------------------------
// sxn Combo-Box
void MainWindow::inSxn_currentIndexChanged(int row)
{
    sxn = sxnList.at(row);

    // read sxn shape
    QString sxnString = AISCshapes->item(row,propList.indexOf("Type"))->text();

    // parse by enum class
    if (sxnString == "W" || sxnString == "M" || sxnString == "S" || sxnString == "HP")
        sxnType = sxnShape::W;
    else if (sxnString == "C" || sxnString == "MC")
        sxnType = sxnShape::C;
    else if (sxnString == "L" || sxnString == "2L")
        sxnType = sxnShape::L;
    else if (sxnString == "WT" || sxnString == "MT" || sxnString == "ST")
        sxnType = sxnShape::WT;
    else if (sxnString == "HSS")
        sxnType = sxnShape::HSS;
    else if (sxnString == "RND" || sxnString == "PIPE")
        sxnType = sxnShape::RND;

    // properties
    theSxn.A = AISCshapes->item(row,propList.indexOf("A"))->text().toDouble();
    theSxn.Ix = AISCshapes->item(row,propList.indexOf("Ix"))->text().toDouble();
    theSxn.Zx = AISCshapes->item(row,propList.indexOf("Zx"))->text().toDouble();
    theSxn.Sx = AISCshapes->item(row,propList.indexOf("Sx"))->text().toDouble();
    theSxn.rx = AISCshapes->item(row,propList.indexOf("rx"))->text().toDouble();
    theSxn.Iy = AISCshapes->item(row,propList.indexOf("Iy"))->text().toDouble();
    theSxn.Zy = AISCshapes->item(row,propList.indexOf("Zy"))->text().toDouble();
    theSxn.Sy = AISCshapes->item(row,propList.indexOf("Sy"))->text().toDouble();
    theSxn.ry = AISCshapes->item(row,propList.indexOf("ry"))->text().toDouble();

    // parse by sxn type
    switch (sxnType)
    {
    case sxnShape::W:
    case sxnShape::M:
    case sxnShape::S:
    case sxnShape::HP:
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("bf/2tf"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("h/tw"))->text().toDouble();
        break;

    case sxnShape::C:
    case sxnShape::MC:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("h/tw"))->text().toDouble();
        break;

    case sxnShape::L:
    case sxnShape::dL:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("b"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("t"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("t"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        break;

    case sxnShape::WT:
    case sxnShape::MT:
    case sxnShape::ST:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        theSxn.d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("bf/2tf"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        break;

    case sxnShape::HSS:
        theSxn.d = AISCshapes->item(row,propList.indexOf("Ht"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("B"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("b/tdes"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("h/tdes"))->text().toDouble();
        break;

    case sxnShape::RND:
    case sxnShape::PIPE:
        theSxn.d = AISCshapes->item(row,propList.indexOf("OD"))->text().toDouble();
        theSxn.bf = AISCshapes->item(row,propList.indexOf("OD"))->text().toDouble();
        theSxn.tw = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.tf = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        theSxn.htw = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        break;

    default:
        theSxn.d = AISCshapes->item(0,propList.indexOf("d"))->text().toDouble();
        theSxn.bf = AISCshapes->item(0,propList.indexOf("bf"))->text().toDouble();
        theSxn.tw = AISCshapes->item(0,propList.indexOf("tw"))->text().toDouble();
        theSxn.tf = AISCshapes->item(0,propList.indexOf("tf"))->text().toDouble();
        theSxn.bftf = AISCshapes->item(0,propList.indexOf("bf/2tf"))->text().toDouble();
        theSxn.htw = AISCshapes->item(0,propList.indexOf("h/tw"))->text().toDouble();
    };

    // call orientation
    inOrient_currentIndexChanged(inOrient->currentIndex());
}
// orientation Combo-Box
void MainWindow::inOrient_currentIndexChanged(int row)
{
    // if round section
    if (sxnType == sxnShape::RND) {
        inNbf->setEnabled(false);
        inNtf->setEnabled(false);
        inNd->setEnabled(true);
        inNtw->setEnabled(true);
        inNbf->setMinimum(0);
        inNtf->setMinimum(0);
        inNbf->setValue(0);
        inNtf->setValue(0);
    } else if (orient != inOrient->itemText(row)) {
        orient = inOrient->itemText(row);
        inNbf->setMinimum(1);
        inNtf->setMinimum(1);

        // switch fibers
        if (orient == "x-x") {
            inNd->setEnabled(true);
            inNtf->setEnabled(true);
            inNd->setValue(nbf);
            inNtf->setValue(ntw);
            inNbf->setValue(1);
            inNtw->setValue(1);
            inNbf->setEnabled(false);
            inNtw->setEnabled(false);

        } else if (orient == "y-y") {
            inNbf->setEnabled(true);
            inNtw->setEnabled(true);
            inNbf->setValue(nd);
            inNtw->setValue(ntf);
            inNtf->setValue(1);
            inNd->setValue(1);
            inNtf->setEnabled(false);
            inNd->setEnabled(false);
        }
    }

    // define section
    if (orient == "x-x")
    {
        theSxn.I = theSxn.Ix;
        theSxn.Z = theSxn.Zx;
        theSxn.S = theSxn.Sx;
        theSxn.r = theSxn.rx;
    }
    else if (orient == "y-y")
    {
        theSxn.I = theSxn.Iy;
        theSxn.Z = theSxn.Zy;
        theSxn.S = theSxn.Sy;
        theSxn.r = theSxn.ry;
    }

    // label
    dlabel->setText(QString("d = %1 in.").arg(theSxn.d));
    dlabel->setToolTip(tr("Depth"));
    bflabel->setText(QString("bf = %1 in.").arg(theSxn.bf));
    bflabel->setToolTip(tr("Flange width"));
    twlabel->setText(QString("tw = %1 in.").arg(theSxn.tw));
    twlabel->setToolTip(tr("Web thickness"));
    tflabel->setText(QString("tf = %1 in.").arg(theSxn.tf));
    tflabel->setToolTip(tr("Flange thickness"));
    Alabel->setText(QString("A = %1 in<sup>2</sup>").arg(theSxn.A));
    Alabel->setToolTip(tr("Cross-sectional area"));
    Ilabel->setText(QString("I = %1 in<sup>4</sup>").arg(theSxn.I));
    Ilabel->setToolTip(tr("Moment of inertia"));
    Zlabel->setText(QString("Z = %1 in<sup>4</sup>").arg(theSxn.Z));
    Zlabel->setToolTip(tr("Plastic section modulus"));
    Slabel->setText(QString("S = %1 in<sup>4</sup>").arg(theSxn.S));
    Slabel->setToolTip(tr("Elastic section modulus"));
    rlabel->setText(QString("r = %1 in<sup>3</sup>").arg(theSxn.r));
    rlabel->setToolTip(tr("Radius of gyration"));

    // to do: user-defined section

    zeroResponse();
}
// element model type
void MainWindow::inElType_currentIndexChanged(int row)
{
    elType = inElType->itemText(row);
    if (elType == "truss")
    {
        inNe->setEnabled(false);
        inNe->setValue(1);
        inNIP->setEnabled(false);
        inNIP->setValue(2);
    } else {
        inNe->setEnabled(true);
        inNIP->setEnabled(true);
    }

    zeroResponse();
}

// element distribution
void MainWindow::inElDist_currentIndexChanged(int row)
{
    elDist = inElDist->itemText(row);
    buildModel();
    if (elDist == "user-defined") {
        QMessageBox::warning(this, "Warning","User-defined nodes not yet implemented.");
        // to do
    }
}

// integration method
void MainWindow::inIM_currentIndexChanged(int row)
{
    IM = inIM->itemText(row);
    zeroResponse();
}

// integration method
void MainWindow::inShape_currentIndexChanged(int row)
{
    shape = inShape->itemText(row);
    buildModel();
}

// material model
void MainWindow::inMat_currentIndexChanged(int row)
{
    mat = inMat->itemText(row);

    switch (inMat->currentIndex()) {
    case 0:
        bBox->setVisible(true);
        steel01Box->setVisible(true);
        steel02Box->setVisible(false);
        steel4Frame->setVisible(false);
        break;

    case 1:
        bBox->setVisible(true);
        steel01Box->setVisible(true);
        steel02Box->setVisible(true);
        steel4Frame->setVisible(false);
        break;

    case 2:
        bBox->setVisible(false);
        steel01Box->setVisible(false);
        steel02Box->setVisible(false);
        steel4Frame->setVisible(true);
        break;
    }

    zeroResponse();
}

// connection-1 model
void MainWindow::in_conn1_currentIndexChanged(int row)
{
    type_conn1 = row;

    if (inclConnSymm == true)
        in_conn2->setCurrentIndex(in_conn1->currentIndex());

    zeroResponse();
}

// connection-2 model
void MainWindow::in_conn2_currentIndexChanged(int row)
{
    type_conn2 = row;
    zeroResponse();
}

void MainWindow::inExp_currentIndexChanged(int row) {
  if (row != -1) {
    loadExperimentalFile(inExp->itemData(row).toString());
  }
}

//---------------------------------------------------------------
// spin-box
// number of elements
void MainWindow::inNe_valueChanged(int var)
{
    // define new
    ne = var;
    buildModel();
}
// number of IPs
void MainWindow::inNIP_valueChanged(int var)
{
    NIP = var;
    buildModel();
}
// fibers across bf
void MainWindow::inNbf_valueChanged(int var)
{
    if (nbf != var){
        nbf = var;
        zeroResponse();
    }
}
// fibers across tf
void MainWindow::inNtf_valueChanged(int var)
{
    if (ntf != var){
        ntf = var;
        zeroResponse();
    }
}
// fibers across d
void MainWindow::inNd_valueChanged(int var)
{
    if (nd != var){
        nd = var;
        zeroResponse();
    }
}
// fibers across tw
void MainWindow::inNtw_valueChanged(int var)
{
    if (ntw != var){
        ntw = var;
        zeroResponse();
    }
}
//---------------------------------------------------------------
// double spin box
// wp length
void MainWindow::inLwp_valueChanged(double var)
{
    if (Lwp != var) {
        Lwp = var;

        buildModel();
    }
}

// brace length
void MainWindow::inL_valueChanged(double var)
{
    if (L != var) {
        L = var;

        buildModel();
    }
}

// camber
void MainWindow::inDelta_valueChanged(double var)
{
    if (delta != var) {
        delta = var/100.0;
        deltaL->setText(QString("                                                        = L/%1").arg(1/(delta)));
        buildModel();
    }
}
// Youngs mod
void MainWindow::inEs_valueChanged(double var)
{
    if (theSteel.Es != var) {
        theSteel.Es = var;
        zeroResponse();
    }
}
// yield strength
void MainWindow::infy_valueChanged(double var)
{
    if (theSteel.fy != var) {
        theSteel.fy = var;
        zeroResponse();
    }
}
// strain hardening
void MainWindow::inb_valueChanged(double var)
{
    if (theSteel.bk != var) {
        theSteel.bk = var;
        inbk->setValue(var);

        // asymm
        if (inclAsymm == false)
            inbkc->setValue(var);

        zeroResponse();
    }
}
// material props
void MainWindow::ina1_valueChanged(double var)
{
    if (theSteel.a1 != var) {
        theSteel.a1 = var;
        zeroResponse();
    }
}
void MainWindow::ina2_valueChanged(double var)
{
    if (theSteel.a2 != var) {
        theSteel.a2 = var;
        zeroResponse();
    }
}
void MainWindow::ina3_valueChanged(double var)
{
    if (theSteel.a3 != var) {
        theSteel.a3 = var;
        zeroResponse();
    }
}
void MainWindow::ina4_valueChanged(double var)
{
    if (theSteel.a4 != var) {
        theSteel.a4 = var;
        zeroResponse();
    }
}
//
void MainWindow::inR0_valueChanged(double var)
{
    if (theSteel.R0k != var) {
        theSteel.R0k = var;
        inR0k->setValue(var);

        // asymm
        if (inclAsymm == false)
            inR0kc->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inR1_valueChanged(double var)
{
    if (theSteel.r1 != var) {
        theSteel.r1 = var;
        inr1->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr1c->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inR2_valueChanged(double var)
{
    if (theSteel.r2 != var) {
        theSteel.r2 = var;
        inr2->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr2c->setValue(var);

        zeroResponse();
    }
}
//
void MainWindow::inbk_valueChanged(double var)
{
    if (theSteel.bk != var) {
        theSteel.bk = var;
        inb->setValue(var);

        // asymm
        if (inclAsymm == false)
            inbkc->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inR0k_valueChanged(double var)
{
    if (theSteel.R0k != var) {
        theSteel.R0k = var;
        inR0->setValue(var);

        // asymm
        if (inclAsymm == false)
            inR0kc->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inr1_valueChanged(double var)
{
    if (theSteel.r1 != var) {
        theSteel.r1 = var;
        inR1->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr1c->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inr2_valueChanged(double var)
{
    if (theSteel.r2 != var) {
        theSteel.r2 = var;
        inR2->setValue(var);

        // asymm
        if (inclAsymm == false)
            inr2c->setValue(var);

        zeroResponse();
    }
}
//
void MainWindow::inbkc_valueChanged(double var)
{
    if (theSteel.bkc != var) {
        theSteel.bkc = var;
        zeroResponse();
    }
}
void MainWindow::inR0kc_valueChanged(double var)
{
    if (theSteel.R0kc != var) {
        theSteel.R0kc = var;
        zeroResponse();
    }
}
void MainWindow::inr1c_valueChanged(double var)
{
    if (theSteel.r1c != var) {
        theSteel.r1c = var;
        zeroResponse();
    }
}
void MainWindow::inr2c_valueChanged(double var)
{
    if (theSteel.r2c != var) {
        theSteel.r2c = var;
        zeroResponse();
    }
}
//
void MainWindow::inbi_valueChanged(double var)
{
    if (theSteel.bi != var) {
        theSteel.bi = var;

        // asymm
        if (inclAsymm == false)
            inbic->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inrhoi_valueChanged(double var)
{
    if (theSteel.rhoi != var) {
        theSteel.rhoi = var;

        // asymm
        if (inclAsymm == false)
            inrhoic->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inbl_valueChanged(double var)
{
    if (theSteel.bl != var) {
        theSteel.bl = var;

        // asymm
        if (inclAsymm == false)
            inblc->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inRi_valueChanged(double var)
{
    if (theSteel.Ri != var) {
        theSteel.Ri = var;

        // asymm
        if (inclAsymm == false)
            inRic->setValue(var);

        zeroResponse();
    }
}
void MainWindow::inlyp_valueChanged(double var)
{
    if (theSteel.lyp != var) {
        theSteel.lyp = var;
        zeroResponse();
    }
}
void MainWindow::inbic_valueChanged(double var)
{
    if (theSteel.bic != var) {
        theSteel.bic = var;
        zeroResponse();
    }
}
void MainWindow::inrhoic_valueChanged(double var)
{
    if (theSteel.rhoic != var) {
        theSteel.rhoic = var;
        zeroResponse();
    }
}
void MainWindow::inblc_valueChanged(double var)
{
    if (theSteel.blc != var) {
        theSteel.blc = var;
        zeroResponse();
    }
}
void MainWindow::inRic_valueChanged(double var)
{
    if (theSteel.Ric != var) {
        theSteel.Ric = var;
        zeroResponse();
    }
}
//
void MainWindow::inm_valueChanged(double var)
{
    if (theFat.m != var) {
        theFat.m = -var;

        zeroResponse();
    }
}
void MainWindow::ine0_valueChanged(double var)
{
    if (theFat.e0 != var) {
        theFat.e0 = var;

        zeroResponse();
    }
}
void MainWindow::inemax_valueChanged(double var)
{
    if (theFat.emax != var) {
        theFat.emax = var;

        zeroResponse();
    }
}
void MainWindow::inemin_valueChanged(double var)
{
    if (theFat.emin != var) {
        theFat.emin = -var;

        zeroResponse();
    }
}

// connection-1
void MainWindow::inl_conn1_valueChanged(double var)
{
    if (conn1.L != var) {
        conn1.L = var;
        inL->setValue(Lwp-conn1.L-conn2.L);

        if (inclConnSymm == true)
            inl_conn2->setValue(var);

        buildModel();
    }
}

void MainWindow::inRigA_conn1_valueChanged(double var)
{
    if (conn1.rigA != var) {
        conn1.rigA = var;

        if (inclConnSymm == true)
            inRigA_conn2->setValue(var);

        zeroResponse();
    }
}

void MainWindow::inRigI_conn1_valueChanged(double var)
{
    if (conn1.rigI != var) {
        conn1.rigI = var;

        if (inclConnSymm == true)
            inRigI_conn2->setValue(var);

        zeroResponse();
    }
}

// connection-2
void MainWindow::inl_conn2_valueChanged(double var)
{
    if (conn2.L != var) {
        conn2.L = var;
        inL->setValue(Lwp-conn1.L-conn2.L);

        buildModel();
    }
}

void MainWindow::inRigA_conn2_valueChanged(double var)
{
    if (conn2.rigA != var) {
        conn2.rigA = var;

        zeroResponse();
    }
}

void MainWindow::inRigI_conn2_valueChanged(double var)
{
    if (conn2.rigI != var) {
        conn2.rigI = var;

        zeroResponse();
    }
}

//---------------------------------------------------------------
// check boxes
void MainWindow::matDefault_checked(int state)
{
    if (state == Qt::Checked) {
        zeroResponse();
        inclDefault = true;

        // set values
        inb->setValue(0.003);
        inbk->setValue(0.003);
        inbi->setValue(0.0025);
        inbl->setValue(0.004);
        //
        ina1->setValue(0);
        ina2->setValue(1.);
        ina3->setValue(0);
        ina4->setValue(1.);
        //
        inR0->setValue(20.);
        inR1->setValue(0.925);
        inR2->setValue(0.15);
        //
        inR0k->setValue(20.);
        inr1->setValue(0.925);
        inr2->setValue(0.15);
        inrhoi->setValue(1.34);
        inRi->setValue(1.0);
        inlyp->setValue(1.0);

        // compression
        if (inclAsymm == true) {

            inbkc->setValue(0.023);
            inbic->setValue(0.0045);
            inblc->setValue(0.004);
            //
            inR0kc->setValue(25.);
            inr1c->setValue(0.9);
            inr2c->setValue(0.15);
            inrhoic->setValue(0.77);
            inRic->setValue(1.0);
        }

        // set enabled
        ina1->setEnabled(false);
        ina2->setEnabled(false);
        ina3->setEnabled(false);
        ina4->setEnabled(false);
        //
        inR0->setEnabled(false);
        inR1->setEnabled(false);
        inR2->setEnabled(false);
        //
        inR0k->setEnabled(false);
        inr1->setEnabled(false);
        inr2->setEnabled(false);
        inrhoi->setEnabled(false);
        inbl->setEnabled(false);
        inRi->setEnabled(false);
        inlyp->setEnabled(false);
        //
        inR0kc->setEnabled(false);
        inr1c->setEnabled(false);
        inr2c->setEnabled(false);
        inrhoic->setEnabled(false);
        inblc->setEnabled(false);
        inRic->setEnabled(false);

    } else {
        inclDefault = false;
        zeroResponse();
        //
        ina1->setEnabled(true);
        ina2->setEnabled(true);
        ina3->setEnabled(true);
        ina4->setEnabled(true);
        //
        inR0->setEnabled(true);
        inR1->setEnabled(true);
        inR2->setEnabled(true);
        //
        inR0k->setEnabled(true);
        inr1->setEnabled(true);
        inr2->setEnabled(true);
        inrhoi->setEnabled(true);
        inbl->setEnabled(true);
        inRi->setEnabled(true);
        inlyp->setEnabled(true);
        //
        if (inclAsymm == true) {
            inR0kc->setEnabled(true);
            inr1c->setEnabled(true);
            inr2c->setEnabled(true);
            inrhoic->setEnabled(true);
            inblc->setEnabled(true);
            inRic->setEnabled(true);
        }
    }
}
void MainWindow::matFat_checked(int state)
{
    if (state == Qt::Checked) {
        inclFat = true;
        fatBox->setVisible(true);

        zeroResponse();

    } else {
        inclFat = false;
        fatBox->setVisible(false);

        zeroResponse();
    }
}
void MainWindow::matAsymm_checked(int state)
{
    if (state == Qt::Checked) {
        inclAsymm = true;
        zeroResponse();

        // set values
        // compression - set values
        inbkc->setValue(0.023);
        inbic->setValue(0.0045);
        inblc->setValue(0.004);
        //
        inR0kc->setValue(25.);
        inr1c->setValue(0.9);
        inr2c->setValue(0.15);
        inrhoic->setValue(0.77);
        inRic->setValue(1.0);

        // set enabled
        inbkc->setEnabled(true);
        inbic->setEnabled(true);

        if (inclDefault == false) {
            inR0kc->setEnabled(true);
            inr1c->setEnabled(true);
            inr2c->setEnabled(true);
            inrhoic->setEnabled(true);
            inblc->setEnabled(true);
            inRic->setEnabled(true);
        }

    } else {
        inclAsymm = false;
        zeroResponse();

        // compression - set values
        inbkc->setValue(inbk->value());
        inbic->setValue(inbi->value());
        //
        inR0kc->setValue(inR0k->value());
        inr1c->setValue(inr1->value());
        inr2c->setValue(inr2->value());
        inrhoic->setValue(inrhoi->value());
        inblc->setValue(inbl->value());
        inRic->setValue(inRi->value());

        // set enabled
        inbkc->setEnabled(false);
        inR0kc->setEnabled(false);
        inr1c->setEnabled(false);
        inr2c->setEnabled(false);
        inbic->setEnabled(false);
        inrhoic->setEnabled(false);
        inblc->setEnabled(false);
        inRic->setEnabled(false);
    }
}

void MainWindow::connSymm_checked(int state)
{
    if (state == Qt::Checked) {
        inclConnSymm = true;
        inl_conn2->setEnabled(false);
        inRigA_conn2->setEnabled(false);
        inRigI_conn2->setEnabled(false);
        in_conn2->setEnabled(false);

        inl_conn2->setValue(inl_conn1->value());
        inRigA_conn2->setValue(inRigA_conn1->value());
        inRigI_conn2->setValue(inRigI_conn1->value());
        in_conn2->setCurrentIndex(in_conn1->currentIndex());

    } else {
        inclConnSymm = false;

        inl_conn2->setEnabled(true);
        inRigA_conn2->setEnabled(true);
        inRigI_conn2->setEnabled(true);
        in_conn2->setEnabled(true);
    }
}
//---------------------------------------------------------------
// slider
void MainWindow::slider_valueChanged(int value)
{
    //pause = true;
    stepCurr = slider->value();

    double Dcurr = (*expD)[value];
    double tcurr = (*time)[value];
    //tlabel->setText(QString("deformation = %1 in.").arg(Dcurr,0,'f',2));

    // update plots
    tPlot->moveDot(tcurr,Dcurr);

    // update deform
    dPlot->plotResponse(value);
    mPlot->plotResponse(value);
    pPlot->plotResponse(value);
    hPlot->plotResponse(value);
}

/*
// press slider
void MainWindow::slider_sliderPressed()
{
    movSlider = true;
}

// released slider
void MainWindow::slider_sliderReleased()
{
    movSlider = false;
}
*/

// stop
/*
void MainWindow::stop_clicked()
{
    stop = true;
}
*/

// play
void MainWindow::play_clicked() {
    if (pause == false) {
        playButton->setText("Play");
        playButton->setToolTip(tr("Play simulation and experimental results"));
        pause = true;
    } else {
        if (playButton->text() == QString("Rewind")) {
            playButton->setText("Play");
            playButton->setToolTip(tr("Play simulation and experimental results"));
            slider->setValue(0);
        } else {
            pause = false;
            playButton->setText("Pause");
            playButton->setToolTip(tr("Pause Results"));
        }
    }
    stepCurr = stepCurr >= numSteps ? 0 : stepCurr;
    
    // play loop
    while (pause == false) {

        slider->setValue(stepCurr);
        QCoreApplication::processEvents();
        stepCurr++;

        if (stepCurr++ == numSteps) {
            pause = true;
            playButton->setText("Rewind");
        }
    };
}

// pause
void MainWindow::pause_clicked()
{
    pause = true;
}

// zero
void MainWindow::restart_clicked()
{
    pause = true;
    playButton->setText("Play");
    playButton->setToolTip(tr("Play simulation and experimental results"));
    slider->setValue(0);
}


void MainWindow::exit_clicked()
{
  QApplication::quit();
}

//---------------------------------------------------------------
void MainWindow::zeroResponse()
{
    // plot original coordinates
    dPlot->setModel(&xc,&yc);
    dPlot->plotModel();
    mPlot->setModel(&xc);
    mPlot->plotModel();
    pPlot->setModel(&xc);
    pPlot->plotModel();

    // re-size response and initialize to zero.
    Ux->reSize(nn,numSteps);
    Uy->reSize(nn,numSteps);

    // force quantities
    q1->reSize(ne,numSteps);
    q2->reSize(ne,numSteps);
    q3->reSize(ne,numSteps);

    dPlot->setResp(Ux,Uy);
    mPlot->setResp(q2,q3);
    pPlot->setResp(q1,q1);

    hPlot->setResp(&(*Ux->data[nn-1]),&(*q1->data[0]));
    //for (int j=0; j<numSteps; j++)
    //    qDebug() << j<< (*Ux->data[nn-1])[j];

    slider->setValue(0);
}

// x coordinates
QVector<double> xCoord(const double L, const int ne, const QString elDist)
{
    // sub-element length
    double l = L/ne;

    // initialize
    QVector<double> x(ne+1);

    // evenly distribute nodes
    for (int j=1; j<=ne; j++) {
        x[j] = x[j-1] + l;
    }

    // take middle 2-3 elements concentrated around mid-length
    if (elDist == "concentrated" && ne > 5) {
        int a = (ne+1)/2-1-0.5*(ne % 2);
        int b = 3 + (ne % 2);
        x = x.mid(a,b);
        x.prepend(0);
        x.append(L);
    }

    // user-defined
    if (elDist == "user-defined") {

    }

    return x;
}

// x coordinates
QVector<double> yCoord(const QVector<double> x, const double p, const QString shape)
{
    int nn = x.size();
    double l = x[nn-1] - x[0];
    QVector<double> y(nn);

    // node closest to midpoint (in case ne = odd)
    double xo = x[nn/2-(nn-1)%2];

    // perturbation shape
    // perturb mid-node
    if (shape == "midpoint perturbation") {
        if (nn != 2) {
            if (nn % 2 == 1) {
                y[nn/2] = p;
            } else {
                y[nn/2-0.5] = p;
                y[nn/2+0.5] = p;
            }
        }

    // arrange in linear shape
    } else if (shape == "linear") {
        // y = p/(L/2)*x if x<L/2
        for (int j=1; j<=nn/2; j++) {
            y[j] = p/xo*x[j];
        }

        // y = 2*p - p/(L/2)*x if x>L/2
        for (int j=ceil(nn/2); j<nn-1; j++) {
            y[j] = l*p/xo-p/xo*x[j];
        }

    // arange nodes in parabolic shape
    } else if (shape == "parabolic") {
        // adjust perturbation to account for ne = odd
        double po = p*l/(4*xo*(1-xo/l));

        // y = 4*p/L*x*(1-x/L)
        for (int j=1; j<nn-1; j++) {
            y[j] = 4*po/l*x[j]*(1-x[j]/l);
        }

    // arrange nodes in sinusoidal shape
    } else if (shape == "sinusoidal") {
        // adjust perturbation to account for ne = odd
        double po = p/(sin(4*atan(1)/l*xo));

        // y = p*sin(pi/L*x);
        for (int j=1; j<nn-1; j++) {
            y[j] = po*sin(4*atan(1)/l*x[j]);
        }
    }

    return y;
}

// fibers
fiberPointer FibRect2D(fiberPointer theFibers, UniaxialMaterial *theMat, const double yi, const double l, const double h, const int nf)
{
    // geometry
    double Af = l*h/nf;
    double dy = l/nf;
    double y0 = (l-dy)/2;

    // add fiber
    for (int j=theFibers.fill, k=0; j<theFibers.fill+nf; j++, k++) {
        double y = yi - y0 + k*dy;
        Fiber *theFiber = new UniaxialFiber2d(j+1,*theMat,Af,y);
        theFibers.data[j] = theFiber;
    }
    theFibers.fill = theFibers.fill + nf;

    return theFibers;
}

double interpolate(QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate)
{
    int size = xData.size();

    // left end of interval
    int j=0;
    if (x >= xData[size-2]) // special case - beyond right end
        j = size-2;
    else
        while (x>xData[j+1])
            j++;

    // points on either side of x
    double xL = xData[j], yL = yData[j], xR = xData[j+1], yR = yData[j+1];

    // if not extrapolating
    if (!extrapolate)
    {
        if (x<xL)
            yR = yL;
        if (x>xR)
            yL = yR;
    }

    // gradient
    double dydx;
    if (xR==xL)
        dydx = 0;
    else
        dydx = (yR-yL)/(xR-xL);

    // linear interpolation
    return yL + dydx*(x - xL);
}

// build model
void MainWindow::buildModel()
{
    pause = true;
    playButton->setText("Play");
    playButton->setToolTip(tr("Play simulation and experimental results"));
    QCoreApplication::processEvents();

    // element lengths
    if (conn1.L < 0.01*Lwp) {
        inl_conn1->setValue(0.01*Lwp);
    }
    if (conn2.L < 0.01*Lwp) {
        inl_conn2->setValue(0.01*Lwp);
    }

    //L = 0.9*Lwp;
    //double Lc1 = 0.05*Lwp;
    //double Lc2 = 0.05*Lwp;

    // camber
    double p = delta*L;

    // node coordinates - should these be pointers?
    // x-coordinates
    xc = xCoord(L, ne, elDist);

    // y-coordinates
    yc = yCoord(xc, p, shape);

    // add nodes for spring elements
    xc.prepend(0); xc.append(L);
    yc.prepend(0); yc.append(0);

    // add nodes for boundary condition elements
    xc.prepend(-conn1.L); xc.append(L+conn2.L);
    yc.prepend(0); yc.append(0);

    // nn
    nn = xc.size();

    // initialize response
    zeroResponse();
}

// Open file
void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
      reset();
      loadFile(fileName);      
    }
    currentFile = fileName;    
}

bool MainWindow::save()
{
    if (currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveAs()
{
    //
    // get filename
    //

    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    // and save the file
    return saveFile(dialog.selectedFiles().first());
}

// Description of this software package that shows in Help menu
void MainWindow::about()
{
    QString textAbout = "\
            <p> This NHERI SimCenter educational application will allow the user to explore how modeling assumptions effect the response of a braced frame element. \
             The application will allow the user to explore effects such as: <ul>\
            <li>number and type of elements, </li> \
            <li>camber and shape of initial imperfection in brace,</li> \
            <li>section discretization,</li> \
            <li> material type and properties,</li> \
            <li>connection details</li></ul> \
            on the response.\
            <p>\
            To allow the user to test validity of their modelling assumptions, the results are compared to data obtained from a number of experimental tests.\
            <p>\
            Developers <ul><li> Main Developer: Professor Barbara Simpson of Oregon State University.</li>\
            <li> Others who have contributed to Coding, Debugging, Testing and Documentation: Frank McKenna, Michael Gardner, and Peter Mackenzie-Helnwein.</li>\
            </ul><p>\
           \
            \
            ";

    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(500, 0, QSizePolicy::Maximum, QSizePolicy::Expanding);
    msgBox.setText(textAbout);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}

// Link to submit feedback through issue on GitHub
void MainWindow::submitFeedback()
{
  QDesktopServices::openUrl(QUrl("https://github.com/NHERI-SimCenter/BracedFrameModeling/issues", QUrl::TolerantMode));
}

// Version this release
void MainWindow::version()
{
    QMessageBox::about(this, tr("Version"),
                       tr("Version 1.0"));
}

// Copyright specification to include in Help menu
void MainWindow::cite()
{
    QString textCite = "\
        <p>\
Barbara Simpson, Frank McKenna, & Michael Gardner. (2018, September 28). \
NHERI-SimCenter BracedFrameModeling (Version v1.0.0). Zenodo. http://doi.org/10.5281/zenodo.1438554 \
      <p>\
      ";


    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    msgBox.setText(textCite);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}



// Copyright specification to include in Help menu
void MainWindow::copyright()
{
    QString textCopyright = "\
        <p>\
        The source code is licensed under a BSD 2-Clause License:<p>\
        \"Copyright (c) 2017-2018, The Regents of the University of California (Regents).\"\
        All rights reserved.<p>\
        <p>\
        Redistribution and use in source and binary forms, with or without \
        modification, are permitted provided that the following conditions are met:\
        <p>\
         1. Redistributions of source code must retain the above copyright notice, this\
         list of conditions and the following disclaimer.\
         \
         \
         2. Redistributions in binary form must reproduce the above copyright notice,\
         this list of conditions and the following disclaimer in the documentation\
         and/or other materials provided with the distribution.\
         <p>\
         THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\
         ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\
         WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\
         DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\
         ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\
         (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\
         LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\
            ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\
            (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\
            SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\
            <p>\
            The views and conclusions contained in the software and documentation are those\
            of the authors and should not be interpreted as representing official policies,\
            either expressed or implied, of the FreeBSD Project.\
            <p>\
            REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, \
            THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\
            THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS \
            PROVIDED \"AS IS\". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,\
            UPDATES, ENHANCEMENTS, OR MODIFICATIONS.\
            <p>\
            ------------------------------------------------------------------------------------\
            <p>\
            The compiled binary form of this application is licensed under a GPL Version 3 license.\
            The licenses are as published by the Free Software Foundation and appearing in the LICENSE file\
            included in the packaging of this application. \
            <p>\
            ------------------------------------------------------------------------------------\
            <p>\
            This software makes use of the QT packages (unmodified): core, gui, widgets and network\
                                                                     <p>\
                                                                     QT is copyright \"The Qt Company Ltd&quot; and licensed under the GNU Lesser General \
                                                                     Public License (version 3) which references the GNU General Public License (version 3)\
      <p>\
      The licenses are as published by the Free Software Foundation and appearing in the LICENSE file\
      included in the packaging of this application. \
      <p>\
      ------------------------------------------------------------------------------------\
      <p>\
      This software makes use of the OpenSees Software Framework. OpenSees is copyright \"The Regents of the University of \
      California\". OpenSees is open-source software whose license can be\
      found at http://opensees.berkeley.edu.\
      <p>\
      ";


    QMessageBox msgBox;
    QSpacerItem *theSpacer = new QSpacerItem(700, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    msgBox.setText(textCopyright);
    QGridLayout *layout = (QGridLayout*)msgBox.layout();
    layout->addItem(theSpacer, layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
}

// build model
void MainWindow::doAnalysis()
{
    // running dialog

    QProgressDialog progressDialog("Running Analysis...", "Cancel", 0, INT_MAX, this);
    QProgressBar* bar = new QProgressBar(&progressDialog);
    bar->setRange(0,numSteps);
    bar->setValue(0);
    progressDialog.setBar(bar);
    progressDialog.setMinimumWidth(500);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setValue(0);

    stop = false;

    // clear existing model
    theDomain.clearAll();
    OPS_clearAllUniaxialMaterial();
    ops_Dt = 0.0;

    // orientations
    //Vector x(3); x(0) = 1.0; x(1) = 0.0; x(2) = 0.0;
    //Vector y(3); y(0) = 0.0; y(1) = 1.0; y(2) = 0.0;

    // direction for spring elements
    //ID dir(2); dir[0] = 0; dir[1] = 2;

    // number of nodes
    //int nn = xc.size(); // + 2 for connections

    // pinned constraints
    static Matrix eqPIN(2,2);
    eqPIN.Zero(); eqPIN(0,0)=1.0; eqPIN(1,1)=1.0;
    static ID dofPIN(2);
    dofPIN(0) = 0; dofPIN(1) = 1;

    // fixed constraints
    static Matrix eqFIX(3,3);
    eqFIX.Zero(); eqFIX(0,0)=1.0; eqFIX(1,1)=1.0; eqFIX(2,2)=1.0;
    static ID dofFIX(3);
    dofFIX(0) = 0; dofFIX(1) = 1; dofFIX(2) = 2;

    // nodes
    Node **theNodes = new Node *[nn];
    for (int j = 0; j < nn; j++) {
        // Node(int tag, int ndof, double Crd1, double Crd2, Vector *displayLoc = 0);
        Node *theNode = new Node(j+1, 3, xc[j], yc[j]);
        theNodes[j] = theNode;
        theDomain.addNode(theNode);

        // SP constraints
        if (j==0) {
            for (int dof = 0; dof < 3; dof++) {
                SP_Constraint *theSP = new SP_Constraint(j+1, dof, 0., true);
                theDomain.addSP_Constraint(theSP);
            }
        }
        if (j==nn-1) {
            for (int dof = 1; dof < 3; dof++) {
                SP_Constraint *theSP = new SP_Constraint(j+1, dof, 0., true);
                theDomain.addSP_Constraint(theSP);
            }
        }

        // MP constraints
        if (j==2) {
            if (elType == "truss") {
                MP_Constraint *theMP = new MP_Constraint(j+1, j, eqFIX, dofFIX, dofFIX);
                theDomain.addMP_Constraint(theMP);

            } else if (type_conn1 == 1) {
                MP_Constraint *theMP = new MP_Constraint(j+1, j, eqFIX, dofFIX, dofFIX);
                theDomain.addMP_Constraint(theMP);

            } else if (type_conn1 == 0) {
                MP_Constraint *theMP = new MP_Constraint(j+1, j, eqPIN, dofPIN, dofPIN);
                theDomain.addMP_Constraint(theMP);
            }
        }
        if (j==nn-2) {           
            if (elType == "truss") {
                MP_Constraint *theMP = new MP_Constraint(j, j+1, eqFIX, dofFIX, dofFIX);
                theDomain.addMP_Constraint(theMP);

            } else if (type_conn2 == 1) {
                MP_Constraint *theMP = new MP_Constraint(j, j+1, eqFIX, dofFIX, dofFIX);
                theDomain.addMP_Constraint(theMP);

            } else if (type_conn2 == 0) {
                MP_Constraint *theMP = new MP_Constraint(j, j+1, eqPIN, dofPIN, dofPIN);
                theDomain.addMP_Constraint(theMP);
            }
        }
    }

    // materials
    UniaxialMaterial *theMat = 0;
    switch (inMat->currentIndex()) {
    case 0:
        // Steel01 (int tag, double fy, double E0, double b, double a1, double a2, double a3, double a4)
        theMat = new Steel01(1, theSteel.fy, theSteel.Es,
                             theSteel.bk, // kin
                             theSteel.a1, theSteel.a2, theSteel.a3, theSteel.a4); // iso
        break;

    case 1:
        // Steel02 (int tag, double fy, double E0, double b, double R0, double cR1, double cR2, double a1, double a2, double a3, double a4)
        theMat = new Steel02(1, theSteel.fy, theSteel.Es,
                             theSteel.bk, theSteel.R0k, theSteel.r1, theSteel.r2, // kin
                             theSteel.a1, theSteel.a2, theSteel.a3, theSteel.a4); // iso

        // double sigInit =0.0);
        break;

    case 2:
        // Steel4 ($matTag $f_y $E_0 < -asym > < -kin $b_k $R_0 $r_1 $r_2 < $b_kc $R_0c $r_1c $r_2c > > < -iso $b_i $rho_i $b_l $R_i $l_yp < $b_ic $rho_ic $b_lc $R_ic> > < -ult $f_u $R_u < $f_uc $R_uc > > < -init $sig_init > < -mem $cycNum >)
        theMat = new Steel4(1,theSteel.fy,theSteel.Es,
                            theSteel.bk, theSteel.R0k, theSteel.r1, theSteel.r2, // kin
                            theSteel.bkc, theSteel.R0kc, theSteel.r1c, theSteel.r2c,
                            theSteel.bi, theSteel.rhoi, theSteel.bl, theSteel.Ri, theSteel.lyp, // iso
                            theSteel.bic, theSteel.rhoic, theSteel.blc, theSteel.Ric,
                            100000000.0*theSteel.fy,50.,100000000.0*theSteel.fy,50., //ult
                            50,0.0); // cycNum + sig_0
        break;

    default:
        QMessageBox::warning(this, "Warning","Material not defined.");
        return;
    }

    // fatigue
    double Dmax = 1.0;
    UniaxialMaterial *theFatMat = new FatigueMaterial(2, *theMat, Dmax,
                            theFat.e0, theFat.m, theFat.emin, theFat.emax);

    // include Fat?
    UniaxialMaterial *theMatIncl;
    if (inclFat == true)
        theMatIncl = theFatMat->getCopy();
    else
        theMatIncl = theMat->getCopy();

    // fibers
    fiberPointer theFibers;
    int nf = 0;

    // UniaxialFiber2d (int tag, UniaxialMaterial &theMat, double Area, double position);
    switch (sxnType)
    {
    case sxnShape::W:
    case sxnShape::M:
    case sxnShape::S:
    case sxnShape::HP:
        if (orient == "x-x") {
            double d0 = theSxn.d - 2*theSxn.tf;
            nf = nd + 2*ntf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // FiberRect2D(theFibers, theMat, yi= patch center, l= length, h= height, nf per patch)
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMatIncl, -(d0+theSxn.tf)/2,theSxn.tf,theSxn.bf,ntf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMatIncl, (d0+theSxn.tf)/2,theSxn.tf,theSxn.bf,ntf);
            // web
            theFibers = FibRect2D(theFibers, theMatIncl, 0,d0,theSxn.tw,nd);
        }
        else
        {
            double d0 = theSxn.d - 2*theSxn.tf;
            nf = ntw + 2*nbf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            //double a = (-(2*bf*tf+d*tw)+sqrt(pow(2*bf*tf+d*tw,2)-4*2*tf*tw*A))/(-2*2*tf*tw);
            //qDebug() << "MODIFY" << a;

            // add fibers
            // FiberRect2D(theFibers, theMat, yi= patch center, l= length, h= height, nf per patch)
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMatIncl, 0,theSxn.bf,theSxn.tf,nbf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMatIncl, 0,theSxn.bf,theSxn.tf,nbf);
            // web
            theFibers = FibRect2D(theFibers, theMatIncl, 0,theSxn.tw,d0,ntw);
        }
        break;

    case sxnShape::C:
    case sxnShape::MC:
    case sxnShape::L:
    case sxnShape::dL:
    case sxnShape::WT:
    case sxnShape::MT:
    case sxnShape::ST:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        return;
        break;

    case sxnShape::HSS:
        if (orient == "x-x") {
            double d0 = theSxn.d - 2*theSxn.tf;
            //double b0 = bf - 2*tw;
            nf = 2*nd + 2*ntf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // FiberRect2D(theFibers, theMat, yi= patch center, l= length, h= height, nf per patch)
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMatIncl, -(d0+theSxn.tf)/2,theSxn.tf,theSxn.bf,ntf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMatIncl, (d0+theSxn.tf)/2,theSxn.tf,theSxn.bf,ntf);
            // web
            theFibers = FibRect2D(theFibers, theMatIncl, 0,d0,theSxn.tw,nd);
            // web
            theFibers = FibRect2D(theFibers, theMatIncl, 0,d0,theSxn.tw,nd);
        }
        else
        {
            //double d0 = d - 2*tf;
            double b0 = theSxn.bf - 2*theSxn.tw;
            nf = 2*ntw + 2*nbf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMatIncl, 0,b0,theSxn.tf,nbf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMatIncl, 0,b0,theSxn.tf,nbf);
            // web
            theFibers = FibRect2D(theFibers, theMatIncl, -(b0+theSxn.tw)/2,theSxn.tw,theSxn.d,ntw);
            // web
            theFibers = FibRect2D(theFibers, theMatIncl, (b0+theSxn.tw)/2,theSxn.tw,theSxn.d,ntw);
        }
        break;

    case sxnShape::RND:
    case sxnShape::PIPE:
        nf = ntw*nd;
        theFibers.data = new Fiber *[nf];
        theFibers.fill = nf;

        // geometry
        double dr = theSxn.tw/ntw;
        double dt = 2*pi/nd;
        double r0 = theSxn.d-theSxn.tw;

        // add fibers
        for (int j=0, k=0; j<ntw; j++) {
            double Af = (pow(r0+(j+1)*dr,2)-pow(r0+j*dr,2))*dt/2;

            for (int m=0; m<nd; m++, k++) {
                double r = r0 + dr/2 + j*dr;
                double t = m*dt;
                double y = r*cos(t);
                Fiber *theFiber = new UniaxialFiber2d(k+1,*theMatIncl,Af,y);
                theFibers.data[k] = theFiber;
            }
        }
        break;
    };

    // geometric transformation
    // CorotCrdTransf2d (int tag, const Vector &rigJntOffsetI, const Vector &rigJntOffsetJ)
    static Vector rigJnt(2); rigJnt.Zero();
    CrdTransf *theTransf = new CorotCrdTransf2d(1,rigJnt,rigJnt);

    // integration types
    //if (IM == )
    BeamIntegration *theIntegration = new LobattoBeamIntegration();

    // sections
    SectionForceDeformation **theSections = new SectionForceDeformation *[NIP];
    // add section
    for (int j=0; j < NIP; j++) {
        SectionForceDeformation *theSection = new FiberSection2d(1, nf, theFibers.data);
        theSections[j] = theSection;
    }

    /*
    // Print flexibility
    Matrix F = theSection[0].getInitialTangent();
    for (int j = 0; j<6; j++) {
        for (int k = 0; k<6; k++) {
            qDebug() << F(j,k);
        }
    }
    */

    // define main elements
    Element **theEls = new Element *[ne];
    double xi[10];

    for (int j=0, k=0; j<ne+4; j++) {

        if (j==0) {
            Element *theEl = new ElasticBeam2d(j+1, conn1.rigA*theSxn.A, theSteel.Es, conn1.rigI*theSxn.I, j+1, j+2, *theTransf);
            theDomain.addElement(theEl);
            //theEls[k] = theEl;
            //theEl->Print(opserr);
            //k++;

        } else if (j==ne+3) {
            Element *theEl = new ElasticBeam2d(j+1, conn2.rigA*theSxn.A, theSteel.Es, conn2.rigI*theSxn.I, j+1, j+2, *theTransf);
            theDomain.addElement(theEl);
            //theEls[k] = theEl;
            //theEl->Print(opserr);
            //k++;

        } else if (j==1) {
            /*
            // spring i
            theMatConn = new Steel01(2,fy,Es,b);
            Element *theEl = new ZeroLength(j+1, 2, j+1, j+2, x, y, theMatConn, dir, 0);
            theDomain.addElement(theEl);
            delete theMatConn; // each ele makes it's own copy
            */

        } else if (j==ne+2) {
            /*
            // spring j
            theMatConn = new Steel01(3,fy,Es,b);
            Element *theEl = new ZeroLength(j+1, 2, j+1, j+2, x, y, theMatConn, dir, 0);
            theDomain.addElement(theEl);
            delete theMatConn; // each ele makes it's own copy
            */

        } else {
            Element *theEl;

            // ForceBeamColumn2d (int tag, int nodeI, int nodeJ, int numSections, SectionForceDeformation **sec, BeamIntegration &beamIntegr, CrdTransf2d &coordTransf, double rho=0.0, int maxNumIters=10, double tolerance=1.0e-12)
            switch (inElType->currentIndex()) {
            case 0:
                theEl = new ForceBeamColumn2d(j+1, j+1, j+2, NIP, theSections, *theIntegration, *theTransf);
                theDomain.addElement(theEl);
                theEls[k] = theEl;
                k++;
                //theEl->Print(opserr);

                // IP locations
                theIntegration->getSectionLocations(NIP,1,xi);

                /*
                // recorders
                theResp[j] = "basicForces";
                theResp[j] = "plasticDeformation";
                theResp[j] = "basicDeformation"; // eps, theta1, theta2
                theResp[j] = "integrationPoints";
                theResp[j] = "section";

                // truss
                theResp[j] = "material";
                */

                break;

            case 1:
                // DispBeamColumn2d (int tag, int nd1, int nd2, int numSections, SectionForceDeformation **s, BeamIntegration &bi, CrdTransf2d &coordTransf, double rho=0.0)
                theEl = new DispBeamColumn2d(j+1, j+1, j+2, NIP, theSections, *theIntegration, *theTransf);
                theDomain.addElement(theEl);
                theEls[k] = theEl;
                k++;
                //theEl->Print(opserr);

                // IP locations
                theIntegration->getSectionLocations(NIP,1,xi);

                break;

            case 2:
                // CorotTruss (int tag, int dim, int Nd1, int Nd2, UniaxialMaterial &theMaterial, double A, double rho=0.0)
                theEl = new CorotTruss (j+1, 2, j+1, j+2, *theMatIncl, theSxn.A);
                theDomain.addElement(theEl);
                theEls[k] = theEl;
                k++;
                //theEl->Print(opserr);

                // IP locations
                xi[0] = 0; xi[1] = 1;

                break;
            }
        }
    }

    // get fiber locations
    double *yf = new double[nf];
    for (int j=0; j<nf; j++) {
        double yLoc, zLoc;
        theFibers.data[j]->getFiberLocation(yLoc, zLoc);
        yf[j] = yLoc;
    }

    // recorders
    int argc =  1;
    const char **argv = new const char *[argc];

    // pull basic force
    char text1[] = "basicForces";
    argv[0] = text1;
    Response **theBasicForce = new Response *[ne];
    for (int j=0; j<ne; j++) {
        theBasicForce[j]  = theEls[j]->setResponse(argv, argc, opserr);
    }

    // plastic def - epsP; theta1P; theta2p
    char text2[] = "plasticDeformation";
    argv[0] = text2;
    Response **thePlasticDef = new Response *[ne];
    for (int j=0; j<ne; j++) {
        thePlasticDef[j]  = theEls[j]->setResponse(argv, argc, opserr);
    }

    // section resp - GaussPointOutput; number; eta

    /*
    argc =  3;
    const char **argv = new const char *[argc];
    char text3[] = "sectionX";
    char text4[] = "fiber";
    //char text5[] = "stressStrain";
    //char text6[] = "deformation"; // axial strain/curv
    argv[0] = text3;
    argv[1] = text4;
    Response **theSxn = new Response *[ne];
    for (int j=0; j<ne; j++) {
        for (int k=0; k<NIP; k++)
            for (int l=0; l<nf; l++)
            {
                //argv[1] = k;
                theSxn[j]  = theEls[j]->setResponse(argv, argc, opserr);
            }
            */

    /*
    // recorders
    theResp[j] = "basicForces";
    theResp[j] = "plasticDeformation";
    theResp[j] = "basicDeformation"; // eps, theta1, theta2
    theResp[j] = "integrationPoints";
    theResp[j] = "section";
    */

    // clean-up
    delete theMat;
    delete theFatMat;
    delete theMatIncl;
    delete [] theFibers.data;
    delete [] theSections;
    delete theIntegration;
    delete theTransf;

    // load pattern
    //PathTimeSeries *theSeries = new PathTimeSeries(1,*expP,*time);
    LinearSeries *theSeries = new LinearSeries(1);
    LoadPattern *theLoadPattern = new LoadPattern(1);
    theLoadPattern->setTimeSeries(theSeries);

    // nodal load
    static Vector load(3); load.Zero(); load(0) = 1;
    NodalLoad *theLoad = new NodalLoad(1,nn,load);
    theLoadPattern->addNodalLoad(theLoad);

    // add to domain
    theDomain.addLoadPattern(theLoadPattern);

    // initial parameters
    double tol0 = 1.0e-8;
    double dU0 = 0.0;

    // solution algorithms
    //EquiSolnAlgo *theNewton = new NewtonRaphson();
    EquiSolnAlgo *theKrylov = new KrylovNewton();
    //EquiSolnAlgo *theLineSearch = new NewtonLineSearch();

    // analysis parameters
    AnalysisModel       *theModel = new AnalysisModel();
    CTestEnergyIncr     *theTest = new CTestEnergyIncr(tol0, 25, 0);
    StaticIntegrator    *theIntegr = new DisplacementControl(nn, 0, dU0, &theDomain, 1, dU0, dU0);
    ConstraintHandler   *theHandler = new PlainHandler();
    RCM                 *theRCM = new RCM();
    DOF_Numberer        *theNumberer = new DOF_Numberer(*theRCM);
#ifdef _FORTRAN_LIBS
    BandGenLinSolver    *theSolver = new BandGenLinLapackSolver();
    LinearSOE           *theSOE = new BandGenLinSOE(*theSolver);
#else
    ProfileSPDLinSolver *theSolver = new ProfileSPDLinDirectSolver();
    LinearSOE           *theSOE = new ProfileSPDLinSOE(*theSolver);
#endif

    theDomain.record();

    // initialize analysis
    StaticAnalysis *theAnalysis = new StaticAnalysis (
        theDomain,*theHandler,*theNumberer,*theModel,*theKrylov,*theSOE,*theIntegr);
    theKrylov->setConvergenceTest(theTest);
    int ok = theAnalysis->analyze(1);
    //theDomain.Print(opserr);

    // re-size response and initialize to zero.
    zeroResponse();

    // initialize
    double tol = tol0;
    double dT0 = 0.01;
    double dT = dT0;
    double tcurr = 0.;
    double tfinal = (*time)[numSteps-1];

    // convergence parameters
    int numConv = 20;
    int tConv = 0;
    int t = 0;

    while (tcurr <= tfinal && stop == false)
    {
        double Ucurr = theNodes[nn-1]->getDisp()(0);
        double U = interpolate(*time, *expD, tcurr+dT, true);
        double dU = U-Ucurr;

        // set up integrator
        StaticIntegrator *theIntegrMod = new DisplacementControl(nn, 0, dU, &theDomain, 1, dU, dU);
        CTestEnergyIncr *theTestMod = new CTestEnergyIncr(tol, 25, 0);

        // analysis
        theAnalysis->setIntegrator(*theIntegrMod);
        theKrylov->setConvergenceTest(theTestMod);
        ok = theAnalysis->analyze(1);

        while (ok != 0 && stop == false)
        {
            // reset number of converged steps
            tConv = 0;
            // check cancelling
            if (progressDialog.wasCanceled())
                stop = true;

            // cut time step
            if (fabs(dT) > 1.0e-4) {
                dT = dT/2;
                qDebug() << "cut dT" << dT;
            }
            else {
                // increase tol (if dT is too small)
                dT = 1.0e-4;
                tol = tol*10;
                qDebug() << "incr tol" << tol;
            }

            // find dU
            double U = interpolate(*time, *expD, tcurr+dT, true);
            double dU = U-Ucurr;

            // modify step/tol
            StaticIntegrator *theIntegrTrial = new DisplacementControl(nn, 0, dU, &theDomain, 1, dU, dU);
            CTestEnergyIncr *theTestTrial = new CTestEnergyIncr(tol, 1000, 0);

            // analysis
            theAnalysis->setIntegrator(*theIntegrTrial);
            theKrylov->setConvergenceTest(theTestTrial);
            ok = theAnalysis->analyze(1);

            // convergence failure
            if (tol > 1.0e-2 && ok != 0)
            {
                stop = true;
                QMessageBox::warning(this, tr("Application"),
                                     tr("Analysis Failed. Results truncated accordingly."));
            }
        }

        // reset
        if (ok == 0)
        {
            // store time
            tcurr = tcurr + dT;
            // check cancelling
            if (progressDialog.wasCanceled())
                stop = true;

            if (tcurr >= (*time)[t])
            {
                progressDialog.setValue(progressDialog.value()+1);

                // store node response
                for (int j=0; j<nn; j++) {
                    const Vector &nodeU = theNodes[j]->getDisp();
                    (*Ux->data[j])[t] = nodeU(0);
                    (*Uy->data[j])[t] = nodeU(1);
                }
                // store basic force
                for (int j=0; j<ne; j++) {
                    //const Vector &q = theEls[j]->getResistingForce();
                    //(*q1->data[j])[t] = -q(0); // N1
                    //(*q2->data[j])[t] = q(2); // M1
                    //(*q3->data[j])[t] = -q(5); // M2

                    theBasicForce[j]->getResponse();
                    Information &info = theBasicForce[j]->getInformation();

                    if (elType == "truss") {
                        double theDouble = info.theDouble;
                        (*q1->data[j])[t] = theDouble;

                    } else {
                        Vector *theVector = info.theVector;
                        (*q1->data[j])[t] = (*theVector)[0];
                        (*q2->data[j])[t] = (*theVector)[1];
                        (*q3->data[j])[t] = -(*theVector)[2];
                    }
                }
                t++;
            }

            // reset
            tConv++;

            // reset tol
            if (tConv >= numConv && tol/10 >= tol0) {
                tConv = 0;
                tol = tol/10;
                qDebug() << "cut tol" << dT;
            }

            // reset dT
            if (tConv >= numConv && fabs(dT)*1.1 <= fabs(dT0)) {
                tConv = 0;
                dT = dT*1.1;
                qDebug() << "increase dT" << dT;
            }
        }
    }

    // set plot
    dPlot->setResp(Ux,Uy);
    pPlot->setResp(q1,q1);
    mPlot->setResp(q2,q3);
    hPlot->setResp(&(*Ux->data[nn-1]),&(*q1->data[0]));

    hPlot->plotResponse(0);

    // close the dialog.
    progressDialog.close();

    // clean-up
    delete [] yf;
    delete [] theNodes;
    delete [] theEls;
    delete [] argv;
    for (int j=0; j<ne; j++)
         delete theBasicForce[j];
    delete [] theBasicForce;
    delete theAnalysis;
}

void MainWindow::setExp(Experiment *exp)
{
    numSteps = exp->getNumSteps();

    // re-size
    time->resize(numSteps);
    expP->resize(numSteps);
    expD->resize(numSteps);

    // extract from experiment
    time = exp->getTime();
    dt = exp->getdt();
    expP = exp->getDataP();
    expD = exp->getDataD();

    // update experiment plot
    tPlot->setData(expD,time);
    hPlot->setModel(expD,expP);
    hPlot->plotModel();

    // zero response
    zeroResponse();

    // set slider
    slider->setRange(0,numSteps-1);
    slider->setValue(0);
}

// layout functions
// header

void MainWindow::createHeaderBox()
{
    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(tr("Braced Frame Modeling"));
    largeLayout->addWidget(header);
}

// footer
void MainWindow::createFooterBox()
{
    FooterWidget *footer = new FooterWidget();
    largeLayout->addWidget(footer);
}

void setLimits(QDoubleSpinBox *widget, int min, int max, int decimal = 0, double step = 1)
{
    widget->setMinimum(min);
    widget->setMaximum(max);
    widget->setDecimals(decimal);
    widget->setSingleStep(step);
}

void setLimits(QSpinBox *widget, int min, int max, double step = 1)
{
    widget->setMinimum(min);
    widget->setMaximum(max);
    widget->setSingleStep(step);
}

// input panel
void MainWindow::createInputPanel()
{
    // units
    QString blank(tr(" "));
    QString kips(tr("k"  ));
    QString g(tr("g"  ));
    QString kipsInch(tr("k/in "));
    QString inch(tr("in. "));
    QString sec(tr("sec "));
    QString percent(tr("\% "));
    QString ksi(tr("ksi "));

    // Lists
    QStringList expList = { "" };
    QStringList elTypeList = { "force-based fiber", "displacement-based fiber", "truss" };
    QStringList distList = { "distributed", "concentrated", "user-defined" };
    QStringList IMList = { "Gauss-Lobatto" };
    QStringList shapeList = { "midpoint perturbation", "linear", "parabolic", "sinusoidal" };
    QStringList orientList = { "y-y", "x-x" };
    QStringList matList = { "uniaxial bilinear (steel01)",
                            "uniaxial Giuffre-Menegotto-Pinto (steel02)",
                            "uniaxial asymmetric Giuffre-Menegotto-Pinto (steel4)" };
    QStringList connList = { "pinned",
                             "fixed" };
                           //"spring with bilinear material" };

    // boxes
    QGroupBox *inBox = new QGroupBox("Input");

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    QWidget *elTab = new QWidget;
    QWidget *sxnTab = new QWidget;
    QWidget *matTab = new QWidget;
    QWidget *connTab = new QWidget;
    //QWidget *analyTab = new QWidget;

    // layouts
    inLay = new QVBoxLayout;
    QGridLayout *expLay = new QGridLayout();
    QGridLayout *elLay = new QGridLayout();
    QGridLayout *sxnLay = new QGridLayout();
    QGridLayout *matLay = new QGridLayout();

    // dynamic labels
    deltaL = new QLabel;
    dlabel = new QLabel;
    bflabel = new QLabel;
    twlabel = new QLabel;
    tflabel = new QLabel;
    Alabel = new QLabel;
    Ilabel = new QLabel;
    rlabel = new QLabel;
    Zlabel = new QLabel;
    Slabel = new QLabel;

    QPushButton *addExp = new QPushButton("Add Experiment");
    addExp->setToolTip(tr("Load different experiment"));
    QPushButton *addAISC = new QPushButton("AISC Database");
    addAISC->setToolTip(tr("Choose brace shape from AISC shapes database v15.0"));

    // experiment bar
    inExp = addCombo(tr("Experiment: "),expList,&blank,expLay,0,0);
    inExp->clear();
    inExp->setToolTip(tr("Experiment name"));
    expLay->addWidget(addExp,0,1);
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    //FMK    int width = 0.7*rec.width();
    //FMK inExp->setMinimumWidth(0.6*width/2);
    expLay->setColumnStretch(2,1);

    // element
    // col-1
    inElType = addCombo(tr("Element Model: "),elTypeList,&blank,elLay,0,0);
    inElType->setToolTip(tr("Select element model type"));
    inLwp = addDoubleSpin(tr("Workpoint Length, Lwp: "),&inch,elLay,1,0);
    inLwp->setToolTip(tr("Brace length from workpoint-to-workpoint"));
    inL = addDoubleSpin(tr("Brace Length, L: "),&inch,elLay,2,0);
    inL->setToolTip(tr("Brace unbraced length"));
    inNe = addSpin(tr("Number of Sub-Elements, ne: "),&blank,elLay,3,0);
    inNe->setToolTip(tr("Number of sub-elements along unbraced length"));
    inNIP = addSpin(tr("Nmber of Integration Points, NIP: "),&blank,elLay,4,0);
    inNIP->setToolTip(tr("Number of integration points in each element"));
    inDelta = addDoubleSpin(tr("Camber: "),&percent,elLay,5,0);
    inDelta->setToolTip(tr("Out-of-plane perturbation to initialize bucking"));
    elLay->addWidget(deltaL,6,0);
    // col-2
    inElDist = addCombo(tr("Sub-Ele Distribution: "),distList,&blank,elLay,7,0);
    inElDist->setToolTip(tr("How sub-elements are distributed along unbraced length"));
    inIM = addCombo(tr("Integration Method: "),IMList,&blank,elLay,8,0);
    inIM->setToolTip(tr("Integration method for each sub-element"));
    inShape = addCombo(tr("Camber Shape: "),shapeList,&blank,elLay,9,0);
    inShape->setToolTip(tr("Geometry of initial brace shape"));
    // stretch

     elLay->addWidget(experimentImage, 10, 0, -1, -1, Qt::AlignCenter);
   // elLay->setColumnStretch(1,1);


    // section
    inSxn = addCombo(tr("Section: "),sxnList,&blank,sxnLay,0,0);
    inSxn->setToolTip(tr("Brace shape from AISC database"));
    sxnLay->addWidget(addAISC,0,1);
    inOrient = addCombo(tr("Orientation: "),orientList,&blank,sxnLay,1,0);
    inOrient->setToolTip(tr("Axis of buckling"));
    // fibers
    // col-1
    inNbf = addSpin(tr("# Fiber Flange Width:"),&blank,sxnLay,2,0);
    inNbf->setToolTip(tr("Number of fibers across flange width"));
    inNtf = addSpin(tr("# Fiber Flange Thickness:"),&blank,sxnLay,3,0);
    inNtf->setToolTip(tr("Number of fibers through flange thickness"));
    inNd = addSpin(tr("# Fiber Web Depth:"),&blank,sxnLay,4,0);
    inNd->setToolTip(tr("Number of fibers through web depth"));
    inNtw = addSpin(tr("# Fiber Web Thickness:"),&blank,sxnLay,5,0);
    inNtw->setToolTip(tr("Number of fibers across web thickness"));
    // add parameters
    // col-2


    sxnLay->addWidget(Alabel,1,1);
    sxnLay->addWidget(Ilabel,2,1);
    sxnLay->addWidget(Zlabel,3,1);
    sxnLay->addWidget(Slabel,4,1);
    sxnLay->addWidget(rlabel,5,1);
    // col-3
    sxnLay->addWidget(dlabel,1,2);
    sxnLay->addWidget(bflabel,2,2);
    sxnLay->addWidget(twlabel,3,2);
    sxnLay->addWidget(tflabel,4,2);
    // stretch
    sxnLay->setColumnStretch(3,1);  
  //  sxnLay->addWidget(experimentImage, 6, 0, -1, -1, Qt::AlignCenter);
    
    // material
    QFrame *matFrame = new QFrame;
    QGridLayout *inMatLay = new QGridLayout;
    inMat = addCombo(tr("Material model: "),matList,&blank,inMatLay,0,0,1,2);
    inMat->setToolTip(tr("Steel material model"));
    matFat = addCheck(tr("Include fatigue: "),blank,inMatLay,1,1);
    matFat->setToolTip(tr("Include low-cycle fatigue material model. Model uses modified "
			  "rainflow counting algorithm to accumulate damage. Fiber stress "
			  "becomes zero when fatigue life is exhausted."));
    matDefault = addCheck(tr("Use defaults: "),blank,inMatLay,2,1);
    matDefault->setToolTip(tr("Use default values from OpenSees"));
    // material parameters
    inEs = addDoubleSpin(tr("E: "),&ksi,inMatLay,1,0);
    inEs->setToolTip(tr("Initial stiffness (Young's Modulus)"));
    infy = addDoubleSpin(tr("Fy: "),&ksi,inMatLay,2,0);
    infy->setToolTip(tr("Material Yield strength"));
    matFrame->setLayout(inMatLay);
    matLay->addWidget(matFrame);
    inMatLay->setColumnStretch(2,1);

    // hardening
    bBox = new QGroupBox("Kinematic Hardening");
    QGridLayout *bLay = new QGridLayout();
    inb = addDoubleSpin(tr("b:   "),&blank,bLay,0,0);
    inb->setToolTip(tr("Strain hardening ratio (ratio between post-yield tangent and initial elastic tangent)"));
    bLay->setColumnStretch(1,1);
    bBox->setLayout(bLay);

    // steel01
    // Steel01 (int tag, double fy, double E0, double b, double a1, double a2, double a3, double a4)
    steel01Box = new QGroupBox("Isotropic Hardening");
    QGridLayout *steel01Lay = new QGridLayout();
    ina1 = addDoubleSpin(tr("a1: "),&blank,steel01Lay,0,0);
    ina1->setToolTip(tr("Increase of compression yield envelope as proportion of yield strength after a "
			"plastic strain of (a2 * fy)/E"));
    ina2 = addDoubleSpin(tr("a2: "),&blank,steel01Lay,1,0);
    ina2->setToolTip(tr("Compression plastic yield factor applied in a1"));    
    ina3 = addDoubleSpin(tr("a3: "),&blank,steel01Lay,0,1);
    ina3->setToolTip(tr("Increase of tension yield envelope as proportion of yield strength after a "
			"plastic strain of (a4 * fy)/E"));
    ina4 = addDoubleSpin(tr("a4: "),&blank,steel01Lay,1,1);
    ina4->setToolTip(tr("Tension plastic yield factor applied in a3"));
    steel01Lay->setColumnStretch(1,1);
    steel01Box->setLayout(steel01Lay);

    // steel02
    // Steel02 (int tag, double fy, double E0, double b, double R0, double cR1, double cR2, double a1, double a2, double a3, double a4)
    steel02Box = new QGroupBox("Elastic to hardening transitions");
    QGridLayout *steel02Lay = new QGridLayout();
    inR0 = addDoubleSpin(tr("R0: "),&blank,steel02Lay,0,0);
    inR0->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));
    inR1 = addDoubleSpin(tr("r1: "),&blank,steel02Lay,1,0);
    inR1->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));    
    inR2 = addDoubleSpin(tr("r2: "),&blank,steel02Lay,2,0);
    inR2->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));    
    steel02Lay->setColumnStretch(1,1);
    steel02Box->setLayout(steel02Lay);

    // steel4
    // Steel4 ($matTag $f_y $E_0 < -asym > < -kin $b_k $R_0 $r_1 $r_2 < $b_kc $R_0c $r_1c $r_2c > > < -iso $b_i $rho_i $b_l $R_i $l_yp < $b_ic $rho_ic $b_lc $R_ic> > < -ult $f_u $R_u < $f_uc $R_uc > > < -init $sig_init > < -mem $cycNum >)
    steel4Frame = new QFrame;
    QGridLayout *steel4Lay = new QGridLayout();
    matAsymm = addCheck(blank,tr("Asymmetric"),steel4Lay,0,0);
    matAsymm->setToolTip(tr("Assume non-symmetric behavior to control material response in tension "
			    "and compression with different parameters"));

    // kinematic hardening
    kinBox = new QGroupBox("Kinematic Hardening");
    QHBoxLayout *kinLay = new QHBoxLayout;
    // tension
    QGroupBox *tKinBox = new QGroupBox("Tension");
    QGridLayout *tKinLay = new QGridLayout();
    inbk = addDoubleSpin(tr("b: "),&blank,tKinLay,0,0);
    inbk->setToolTip(tr("Kinematic hardening ratio"));
    inR0k = addDoubleSpin(tr("R0: "),&blank,tKinLay,1,0);
    inR0k->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));    
    inr1 = addDoubleSpin(tr("r1: "),&blank,tKinLay,2,0);
    inr1->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));        
    inr2 = addDoubleSpin(tr("r2: "),&blank,tKinLay,3,0);
    inr2->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));
    tKinLay->setColumnStretch(1,1);
    tKinBox->setLayout(tKinLay);
    kinLay->addWidget(tKinBox);
    // compression
    cKinBox = new QGroupBox("Compression");
    QGridLayout *cKinLay = new QGridLayout();
    inbkc = addDoubleSpin(tr("b: "),&blank,cKinLay,0,0);
    inbkc->setToolTip(tr("Kinematic hardening ratio"));    
    inR0kc = addDoubleSpin(tr("R0: "),&blank,cKinLay,1,0);
    inR0kc->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));        
    inr1c = addDoubleSpin(tr("r1: "),&blank,cKinLay,2,0);
    inr1c->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));        
    inr2c = addDoubleSpin(tr("r2: "),&blank,cKinLay,3,0);
    inr2c->setToolTip(tr("Controls exponential transition from linear elastic to hardening asymptote"));
    cKinLay->setColumnStretch(1,1);
    cKinBox->setLayout(cKinLay);
    kinLay->addWidget(cKinBox);
    kinBox->setLayout(kinLay);

    // isotropic hardening
    isoBox = new QGroupBox("Isotropic Hardening");
    QHBoxLayout *isoLay = new QHBoxLayout;
    // tension
    QGroupBox *tIsoBox = new QGroupBox("Tension");
    QGridLayout *tIsoLay = new QGridLayout();
    inbi = addDoubleSpin(tr("b: "),&blank,tIsoLay,0,0);
    inbi->setToolTip(tr("Initial isotropic hardening ratio"));
    inrhoi = addDoubleSpin(tr("rho: "),&blank,tIsoLay,1,0);
    inrhoi->setToolTip(tr("Position of the intersection point between initial and saturated hardening asymptotes"));
    inbl = addDoubleSpin(tr("bl: "),&blank,tIsoLay,2,0);
    inbl->setToolTip(tr("Saturated hardening ratio"));
    inRi = addDoubleSpin(tr("Ri: "),&blank,tIsoLay,3,0);
    inRi->setToolTip(tr("Controls exponential transition from initial to saturated asymptotes"));
    inlyp = addDoubleSpin(tr("lyp: "),&blank,tIsoLay,4,0);
    inlyp->setToolTip(tr("Length of yield plateau"));
    tIsoLay->setColumnStretch(1,1);
    tIsoBox->setLayout(tIsoLay);
    isoLay->addWidget(tIsoBox);
    // compression
    cIsoBox = new QGroupBox("Compression");
    QGridLayout *cIsoLay = new QGridLayout();
    inbic = addDoubleSpin(tr("b: "),&blank,cIsoLay,0,0);
    inbic->setToolTip(tr("Initial isotropic hardening ratio"));
    inrhoic = addDoubleSpin(tr("rho: "),&blank,cIsoLay,1,0);
    inrhoic->setToolTip(tr("Position of the intersection point between initial and saturated hardening asymptotes"));
    inblc = addDoubleSpin(tr("bl: "),&blank,cIsoLay,2,0);
    inblc->setToolTip(tr("Saturated hardening ratio"));
    inRic = addDoubleSpin(tr("Ri: "),&blank,cIsoLay,3,0);
    inRic->setToolTip(tr("Controls exponential transition from initial to saturated asymptotes"));
    cIsoLay->setColumnStretch(1,1);
    cIsoLay->setRowStretch(4,1);
    cIsoBox->setLayout(cIsoLay);
    isoLay->addWidget(cIsoBox);
    isoBox->setLayout(isoLay);

    // add widgets
    steel4Lay->addWidget(matAsymm,0,0);
    steel4Lay->addWidget(kinBox,1,0,1,3);
    steel4Lay->addWidget(isoBox,2,0,1,3);
    steel4Lay->setColumnStretch(1,1);
    steel4Frame->setLayout(steel4Lay);

    // add layouts
    matLay->addWidget(bBox,4,0,1,3);
    matLay->addWidget(steel02Box,5,0,1,3);
    matLay->addWidget(steel01Box,6,0,1,3);
    matLay->addWidget(steel4Frame,4,0,1,3);

    // fatigue parameters
    fatBox = new QGroupBox("Fatigue");
    QGridLayout *fatLay = new QGridLayout();
    inm = addDoubleSpin(tr("m: -"),&blank,fatLay,0,0);
    inm->setToolTip(tr("Slope of Coffin-Manson curve in log-log space"));
    ine0 = addDoubleSpin(tr("e0: "),&blank,fatLay,1,0);
    ine0->setToolTip(tr("Value of strain at which one cycle will cause failure"));
    inemin = addDoubleSpin(tr("emin: -"),&blank,fatLay,0,1);
    inemin->setToolTip(tr("Global minimum value of strain or deformation"));
    inemax = addDoubleSpin(tr("emax: "),&blank,fatLay,1,1);
    inemax->setToolTip(tr("Global maximim value of strain or deformation"));
    fatLay->setColumnStretch(2,1);
    fatBox->setLayout(fatLay);

    // add layouts
    fatBox->setVisible(true);
    matLay->addWidget(fatBox,7,0,1,3);

    // stretch
    matLay->setColumnStretch(4,1);

    // connections
    QGridLayout *connLay = new QGridLayout();
    //connSymm = addCheck(blank,tr("Symmetric connections"),connLay,0,0);
    QHBoxLayout *connSymLayout = new QHBoxLayout();
    QLabel *labelSymm = new QLabel(tr("Symmetric Connections"));
    connSymm = new QCheckBox();
    connSymLayout->addStretch();
    connSymLayout->addWidget(labelSymm);
    connSymLayout->addWidget(connSymm);

    connSymm->setToolTip(tr("Set Connection-2 to be the same as Connection-1"));
    connLay->addLayout(connSymLayout,0,1);

    // connection-1
    QGroupBox *conn1Box = new QGroupBox("Connection-1");
    QGridLayout *conn1Lay = new QGridLayout();
    in_conn1 = addCombo(tr("Model: "),connList,&blank,conn1Lay,0,0);
    conn1Lay->setColumnStretch(1,1);


    // mat
    /*
    QGroupBox *conn1matBox = new QGroupBox("Material");
    QGridLayout *conn1matLay = new QGridLayout();
    infy_conn1 = addDoubleSpin(tr("fy: "),&ksi,conn1matLay,0,0);
    inEs_conn1 = addDoubleSpin(tr("Es: "),&ksi,conn1matLay,1,0);
    inb_conn1 = addDoubleSpin(tr("b: "),&blank,conn1matLay,2,0);
    conn1matLay->setColumnStretch(1,1);
    conn1matBox->setLayout(conn1matLay);
    conn1Lay->addWidget(conn1matBox,1,0);
    */
    // geom
    QGroupBox *conn1geoBox = new QGroupBox("Gusset geometry");
    QGridLayout *conn1geoLay = new QGridLayout();
    //intg_conn1 = addDoubleSpin(tr("thickness: "),&inch,conn1geoLay,0,0);
    inl_conn1 = addDoubleSpin(tr("length: "),&inch,conn1geoLay,1,0);
    inl_conn1->setToolTip(tr("Length from workpoint to beginning/end of unbraced length"));
    //inlw_conn1 = addDoubleSpin(tr("width: "),&inch,conn1geoLay,2,0);
    conn1geoLay->setColumnStretch(1,1);
    conn1geoBox->setLayout(conn1geoLay);
    conn1Lay->addWidget(conn1geoBox,2,0);
    // rigid end elements
    QGroupBox *conn1rigBox = new QGroupBox("Rigid multiplier");
    QGridLayout *conn1rigLay = new QGridLayout();
    inRigA_conn1 = addDoubleSpin(tr("A: "),&blank,conn1rigLay,0,0);
    inRigA_conn1->setToolTip(tr("Multiplies area of elastic end element to represent relative rigidity of "
				"connection to brace: A<sub>conn</sub>/A<sub>brace</sub>"));
    inRigI_conn1 = addDoubleSpin(tr("I: "),&blank,conn1rigLay,1,0);
    inRigI_conn1->setToolTip(tr("Multiplies moment of inertia of elastic end element to represent relative"
                " rigidity of connection to brace: <sub>Iconn</sub>/I<sub>brace</sub>"));
    conn1rigLay->setColumnStretch(1,1);
    conn1rigBox->setLayout(conn1rigLay);
    conn1Lay->addWidget(conn1rigBox,3,0);
    // add to layout
    conn1Box->setLayout(conn1Lay)
            ;
    connLay->addWidget(conn1Box,1,0);

    // connection-2
    QGroupBox *conn2Box = new QGroupBox("Connection-2");
    QGridLayout *conn2Lay = new QGridLayout();
    in_conn2 = addCombo(tr("Model: "),connList,&blank,conn2Lay,0,0);
    conn2Lay->setColumnStretch(1,1);
    // mat
    /*
    QGroupBox *conn2matBox = new QGroupBox("Material");
    QGridLayout *conn2matLay = new QGridLayout();
    infy_conn2 = addDoubleSpin(tr("fy: "),&ksi,conn2matLay,0,0);
    inEs_conn2 = addDoubleSpin(tr("Es: "),&ksi,conn2matLay,1,0);
    inb_conn2 = addDoubleSpin(tr("b: "),&blank,conn2matLay,2,0);
    conn2matLay->setColumnStretch(1,1);
    conn2matBox->setLayout(conn2matLay);
    conn2Lay->addWidget(conn2matBox,1,0);
    */
    // geom
    QGroupBox *conn2geoBox = new QGroupBox("Gusset geometry");
    QGridLayout *conn2geoLay = new QGridLayout();
    //intg_conn2 = addDoubleSpin(tr("thickness: "),&inch,conn2geoLay,0,0);
    inl_conn2 = addDoubleSpin(tr("length: "),&inch,conn2geoLay,1,0);
    inl_conn2->setToolTip(tr("Length from workpoint to beginning/end of unbraced length"));    
    //inlw_conn2 = addDoubleSpin(tr("width: "),&inch,conn2geoLay,2,0);
    conn2geoLay->setColumnStretch(1,1);
    conn2geoBox->setLayout(conn2geoLay);
    conn2Lay->addWidget(conn2geoBox,2,0);
    // rigid end elements
    QGroupBox *conn2rigBox = new QGroupBox("Rigid multiplier");
    QGridLayout *conn2rigLay = new QGridLayout();
    inRigA_conn2 = addDoubleSpin(tr("A: "),&blank,conn2rigLay,0,0);
    inRigA_conn2->setToolTip(tr("Multiplies area of elastic end element to represent relative rigidity of "
				"connection to brace: A<sub>conn</sub>/A<sub>brace</sub>"));    
    inRigI_conn2 = addDoubleSpin(tr("I: "),&blank,conn2rigLay,1,0);
    inRigI_conn2->setToolTip(tr("Multiplies moment of inertia of elastic end element to represent relative"
                " rigidity of connection to brace: I<sub>conn</sub>/I<sub>brace</sub>"));
    conn2rigLay->setColumnStretch(1,1);
    conn2rigBox->setLayout(conn2rigLay);
    conn2Lay->addWidget(conn2rigBox,3,0);
    // add to layout
    conn2Box->setLayout(conn2Lay);
    connLay->addWidget(conn2Box,1,1);

    // el limits
    setLimits(inLwp, 1, 10000, 2, 1);
    setLimits(inL, 1, 10000, 2, 1);
    inL->setEnabled(false);
    setLimits(inNe, 1, 100);
    setLimits(inNIP, 1, 10);
    setLimits(inDelta, 0, 10000, 3, 0.001);
    // sxn limits
    setLimits(inNbf, 1, 100);
    setLimits(inNtf, 1, 100);
    setLimits(inNd, 1, 100);
    setLimits(inNtw, 1, 100);
    // mat limits
    setLimits(inEs, 1, 100000, 1, 1000);
    setLimits(infy, 1, 200, 1, 1);
    //
    setLimits(inb, 0, 1, 5, 0.00001);
    setLimits(inbk, 0, 1, 5, 0.00001);
    setLimits(inbkc, 0, 1, 5, 0.00001);
    setLimits(inbi, 0, 1, 5, 0.00001);
    setLimits(inbic, 0, 1, 5, 0.00001);
    setLimits(inbl, 0, 1, 5, 0.00001);
    setLimits(inblc, 0, 1, 5, 0.00001);
    //
    setLimits(ina1, 0, 100, 2, 0.1);
    setLimits(ina2, 0, 100, 2, 0.1);
    setLimits(ina3, 0, 100, 2, 0.1);
    setLimits(ina4, 0, 100, 2, 0.1);
    //
    setLimits(inR0, 0, 50, 1);
    setLimits(inR0k, 0, 50, 1);
    setLimits(inR0kc, 0, 50, 1);
    setLimits(inRi, 0, 50, 1);
    setLimits(inRic, 0, 50, 1);
    //
    setLimits(inR1, 0, 1, 3, 0.01);
    setLimits(inR2, 0, 1, 3, 0.01);
    setLimits(inr1, 0, 1, 3, 0.01);
    setLimits(inr2, 0, 1, 3, 0.01);
    setLimits(inr1c, 0, 1, 3, 0.01);
    setLimits(inr2c, 0, 1, 3, 0.01);
    //
    setLimits(inrhoi, 0, 5, 3, 0.01);
    setLimits(inrhoic, 0, 5, 3, 0.01);
    setLimits(inlyp, 0, 10, 3, 0.01);
    //
    setLimits(inm, 0, 100, 3, 0.001);
    setLimits(ine0, 0, 100, 3, 0.001);
    setLimits(inemin, 0, 100, 3, 0.001);
    setLimits(inemax, 0, 100, 3, 0.001);

    // buttons
    // buttons
    QHBoxLayout *buttonLay = new QHBoxLayout();

    QPushButton *run = new QPushButton("Analyze");
    run->setToolTip(tr("Run simulation with current properities"));
    //QPushButton *stop = new QPushButton("stop");
    QPushButton *reset = new QPushButton("Reset");
    reset->setToolTip(tr("Clear simulation results and reload default experiment"));
    playButton = new QPushButton("Play");
    playButton->setToolTip(tr("Play simulation and experimental results"));
   // QPushButton *pause = new QPushButton("Pause");
   // pause->setToolTip(tr("Pause current results playback"));
   //QPushButton *restart = new QPushButton("Restart");
   // restart->setToolTip(tr("Restart results playback"));
    QPushButton *exitApp = new QPushButton("Exit");
    exitApp->setToolTip(tr("Exit Application"));


    buttonLay->addWidget(run);
    buttonLay->addWidget(playButton);
    buttonLay->addWidget(reset);  
   // buttonLay->addWidget(pause);
   // buttonLay->addWidget(restart);
    buttonLay->addWidget(exitApp);
   // buttonLay->setColumnStretch(3,1);

    // set tab layouts
    inLay->addLayout(expLay);
    elTab->setLayout(elLay);
    elLay->setRowStretch(elLay->rowCount(),1);
    sxnTab->setLayout(sxnLay);
    sxnLay->setRowStretch(sxnLay->rowCount(),1);
    matTab->setLayout(matLay);
    matLay->setRowStretch(matLay->rowCount(),1);
    connTab->setLayout(connLay);
    connLay->setRowStretch(connLay->rowCount(),1);

    // add layout to tab
    tabWidget->addTab(elTab, "Element");
    tabWidget->addTab(sxnTab, "Section");
    tabWidget->addTab(matTab, "Material");
    tabWidget->addTab(connTab, "Connection");
    //tabWidget->addTab(analyTab, "Analysis");
    //FMK    tabWidget->setMinimumWidth(0.5*width);

    // add tab
    inLay->addWidget(tabWidget);
    //inLay->addStretch();

    // add buttons
    inLay->addLayout(buttonLay);

    // add to main layout
    inBox->setLayout(inLay);
    mainLayout->addWidget(inBox, 0);

    //largeLayout->addLayout(mainLayout);

    // connect signals / slots
    // buttons
    connect(addExp,SIGNAL(clicked()), this, SLOT(addExp_clicked()));
    connect(addAISC,SIGNAL(clicked()), this, SLOT(addAISC_clicked()));
    connect(reset,SIGNAL(clicked()), this, SLOT(reset()));
    connect(run,SIGNAL(clicked()), this, SLOT(doAnalysis()));
    //connect(stop,SIGNAL(clicked()), this, SLOT(stop_clicked()));
    connect(playButton,SIGNAL(clicked()), this, SLOT(play_clicked()));
    //connect(pause,SIGNAL(clicked()), this, SLOT(pause_clicked()));
    //connect(restart,SIGNAL(clicked()), this, SLOT(restart_clicked()));
    connect(exitApp,SIGNAL(clicked()), this, SLOT(exit_clicked()));

    // Combo Box
    connect(inSxn,SIGNAL(currentIndexChanged(int)), this, SLOT(inSxn_currentIndexChanged(int)));
    connect(inOrient,SIGNAL(currentIndexChanged(int)), this, SLOT(inOrient_currentIndexChanged(int)));
    connect(inElType,SIGNAL(currentIndexChanged(int)), this, SLOT(inElType_currentIndexChanged(int)));
    connect(inElDist,SIGNAL(currentIndexChanged(int)), this, SLOT(inElDist_currentIndexChanged(int)));
    connect(inIM,SIGNAL(currentIndexChanged(int)), this, SLOT(inIM_currentIndexChanged(int)));
    connect(inShape,SIGNAL(currentIndexChanged(int)), this, SLOT(inShape_currentIndexChanged(int)));
    connect(inMat,SIGNAL(currentIndexChanged(int)), this, SLOT(inMat_currentIndexChanged(int)));
    connect(in_conn1,SIGNAL(currentIndexChanged(int)), this, SLOT(in_conn1_currentIndexChanged(int)));
    connect(in_conn2,SIGNAL(currentIndexChanged(int)), this, SLOT(in_conn2_currentIndexChanged(int)));
    connect(inExp, SIGNAL(currentIndexChanged(int)), this, SLOT(inExp_currentIndexChanged(int)));

    // Spin box
    connect(inNe,SIGNAL(valueChanged(int)), this, SLOT(inNe_valueChanged(int)));
    connect(inNIP,SIGNAL(valueChanged(int)), this, SLOT(inNIP_valueChanged(int)));
    connect(inNbf,SIGNAL(valueChanged(int)), this, SLOT(inNbf_valueChanged(int)));
    connect(inNtf,SIGNAL(valueChanged(int)), this, SLOT(inNtf_valueChanged(int)));
    connect(inNd,SIGNAL(valueChanged(int)), this, SLOT(inNd_valueChanged(int)));
    connect(inNtw,SIGNAL(valueChanged(int)), this, SLOT(inNtw_valueChanged(int)));

    // double spin box
    connect(inLwp,SIGNAL(valueChanged(double)), this, SLOT(inLwp_valueChanged(double)));
    connect(inL,SIGNAL(valueChanged(double)), this, SLOT(inL_valueChanged(double)));
    connect(inDelta,SIGNAL(valueChanged(double)), this, SLOT(inDelta_valueChanged(double)));
    //
    connect(inEs,SIGNAL(valueChanged(double)), this, SLOT(inEs_valueChanged(double)));
    connect(infy,SIGNAL(valueChanged(double)), this, SLOT(infy_valueChanged(double)));
    //
    connect(inb,SIGNAL(valueChanged(double)), this, SLOT(inb_valueChanged(double)));
    //
    connect(ina1,SIGNAL(valueChanged(double)), this, SLOT(ina1_valueChanged(double)));
    connect(ina2,SIGNAL(valueChanged(double)), this, SLOT(ina2_valueChanged(double)));
    connect(ina3,SIGNAL(valueChanged(double)), this, SLOT(ina3_valueChanged(double)));
    connect(ina4,SIGNAL(valueChanged(double)), this, SLOT(ina4_valueChanged(double)));
    //
    connect(inR0,SIGNAL(valueChanged(double)), this, SLOT(inR0_valueChanged(double)));
    connect(inR1,SIGNAL(valueChanged(double)), this, SLOT(inR1_valueChanged(double)));
    connect(inR2,SIGNAL(valueChanged(double)), this, SLOT(inR2_valueChanged(double)));
    //
    connect(inbk,SIGNAL(valueChanged(double)), this, SLOT(inbk_valueChanged(double)));
    connect(inR0k,SIGNAL(valueChanged(double)), this, SLOT(inR0k_valueChanged(double)));
    connect(inr1,SIGNAL(valueChanged(double)), this, SLOT(inr1_valueChanged(double)));
    connect(inr2,SIGNAL(valueChanged(double)), this, SLOT(inr2_valueChanged(double)));
    connect(inbkc,SIGNAL(valueChanged(double)), this, SLOT(inbkc_valueChanged(double)));
    connect(inR0kc,SIGNAL(valueChanged(double)), this, SLOT(inR0kc_valueChanged(double)));
    connect(inr1c,SIGNAL(valueChanged(double)), this, SLOT(inr1c_valueChanged(double)));
    connect(inr2c,SIGNAL(valueChanged(double)), this, SLOT(inr2c_valueChanged(double)));
    //
    connect(inbi,SIGNAL(valueChanged(double)), this, SLOT(inbi_valueChanged(double)));
    connect(inrhoi,SIGNAL(valueChanged(double)), this, SLOT(inrhoi_valueChanged(double)));
    connect(inbl,SIGNAL(valueChanged(double)), this, SLOT(inbl_valueChanged(double)));
    connect(inRi,SIGNAL(valueChanged(double)), this, SLOT(inRi_valueChanged(double)));
    connect(inlyp,SIGNAL(valueChanged(double)), this, SLOT(inlyp_valueChanged(double)));
    connect(inbic,SIGNAL(valueChanged(double)), this, SLOT(inbic_valueChanged(double)));
    connect(inrhoic,SIGNAL(valueChanged(double)), this, SLOT(inrhoic_valueChanged(double)));
    connect(inblc,SIGNAL(valueChanged(double)), this, SLOT(inblc_valueChanged(double)));
    connect(inRic,SIGNAL(valueChanged(double)), this, SLOT(inRic_valueChanged(double)));
    //
    connect(inm,SIGNAL(valueChanged(double)), this, SLOT(inm_valueChanged(double)));
    connect(ine0,SIGNAL(valueChanged(double)), this, SLOT(ine0_valueChanged(double)));
    connect(inemin,SIGNAL(valueChanged(double)), this, SLOT(inemin_valueChanged(double)));
    connect(inemax,SIGNAL(valueChanged(double)), this, SLOT(inemax_valueChanged(double)));
    //
    connect(inl_conn1,SIGNAL(valueChanged(double)), this, SLOT(inl_conn1_valueChanged(double)));
    connect(inRigA_conn1,SIGNAL(valueChanged(double)), this, SLOT(inRigA_conn1_valueChanged(double)));
    connect(inRigI_conn1,SIGNAL(valueChanged(double)), this, SLOT(inRigI_conn1_valueChanged(double)));
    //
    connect(inl_conn2,SIGNAL(valueChanged(double)), this, SLOT(inl_conn2_valueChanged(double)));
    connect(inRigA_conn2,SIGNAL(valueChanged(double)), this, SLOT(inRigA_conn2_valueChanged(double)));
    connect(inRigI_conn2,SIGNAL(valueChanged(double)), this, SLOT(inRigI_conn2_valueChanged(double)));

    // check box
    connect(matFat, SIGNAL(stateChanged(int)), this, SLOT(matFat_checked(int)));
    connect(matDefault, SIGNAL(stateChanged(int)), this, SLOT(matDefault_checked(int)));
    connect(matAsymm, SIGNAL(stateChanged(int)), this, SLOT(matAsymm_checked(int)));
    connect(connSymm, SIGNAL(stateChanged(int)), this, SLOT(connSymm_checked(int)));
}

// output panel
void MainWindow::createOutputPanel()
{
    //
    // deformed shape
    //

    /* FMK  moving to the tab widget
    QGroupBox *dispBox = new QGroupBox("Deformed Shape");
    QGridLayout *dispLay = new QGridLayout();
    dPlot = new deformWidget(tr("Length, Lwp"), tr("Deformation"));
    dispLay->addWidget(dPlot,0,0);
    dispBox->setLayout(dispLay);
    */


    //
    // Applied History - loading plot & slider
    //   - placed in a group box
    //

    QGroupBox *tBox = new QGroupBox("Applied Displacement History");
    QVBoxLayout *tLay = new QVBoxLayout();

    // loading plot
    tPlot = new historyWidget(tr("Pseudo-time"), tr("Applied History"));
    tLay->addWidget(tPlot);

    // slider
    slider = new QSlider(Qt::Horizontal);
    tLay->addWidget(slider);

    tBox->setLayout(tLay);

    //
    // hysteretic plot
    //

    QGroupBox *hystBox = new QGroupBox("Hysteretic Response");
    QVBoxLayout *hystLay = new QVBoxLayout();
    hPlot = new hysteresisWidget(tr("Axial Deformation [in.]"), tr("Axial Force [kips]"));
    hystLay->addWidget(hPlot,1);
    hystBox->setLayout(hystLay);

    //
    // Tab Widget containing axial, moment & soon to be curvature!
    //

    QTabWidget *tabWidget = new QTabWidget(this);

    // deformed shape plot
    dPlot = new deformWidget(tr("Length, Lwp"), tr("Deformation"));
    tabWidget->addTab(dPlot, "Displaced Shape");

    // axial force plot
    pPlot = new responseWidget(tr("Length, Lwp"), tr("Axial Force"));
    tabWidget->addTab(pPlot, "Axial Force Diagram");

    // moment plot
    mPlot = new responseWidget(tr("Length, Lwp"), tr("Moment"));
    tabWidget->addTab(mPlot, "Moment Diagram");

    // curvature plot
    // TO DO

    //
    // main output box
    //
    QGroupBox *outBox = new QGroupBox("Output");
    QVBoxLayout *outputLayout = new QVBoxLayout();

    /* FMK - moving disp to tab
    QHBoxLayout *dispAndTabLayout = new QHBoxLayout();
    dispAndTabLayout->addWidget(dispBox,1);
    dispAndTabLayout->addWidget(tabWidget,1);
    outputLayout->addLayout(dispAndTabLayout,0.2);
    */
    outputLayout->addWidget(tabWidget,2);


    outputLayout->addWidget(hystBox,6);
    outputLayout->addWidget(tBox,2);


  //  outLay = new QGridLayout;
  //  outLay->addWidget(dispBox,0,0);
  //  outLay->addWidget(tabWidget,1,0);
  //  outLay->addWidget(hystBox,0,1,2,1);
  //  outLay->addWidget(tBox,2,0,1,2);

  // outLay->addLayout(buttonLay,3,0);

    // add to main layout
    outBox->setLayout(outputLayout);
    mainLayout->addWidget(outBox,1);

    //connect(slider, SIGNAL(sliderPressed()),  this, SLOT(slider_sliderPressed()));
    //connect(slider, SIGNAL(sliderReleased()), this, SLOT(slider_sliderReleased()));
    connect(slider, SIGNAL(valueChanged(int)),this, SLOT(slider_valueChanged(int)));
}

// Create actions for File and Help menus
void MainWindow::createActions() {

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAction = new QAction(tr("&Open"), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction(tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document to disk"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("&Save As"), this);
    saveAction->setStatusTip(tr("Save the document with new filename to disk"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *infoAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    QAction *submitAct = helpMenu->addAction(tr("&Provide Feedback"), this, &MainWindow::submitFeedback);
    QAction *aboutAct = helpMenu->addAction(tr("&Version"), this, &MainWindow::version);
    QAction *citeAct = helpMenu->addAction(tr("&How to Cite"), this, &MainWindow::cite);
    QAction *copyrightAct = helpMenu->addAction(tr("&License"), this, &MainWindow::copyright);

}


// label functions
// name(QLabel) + enter(QComboBox) + units(QLabel)
QCheckBox *addCheck(QString text, QString unitText,
                    QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QCheckBox *res = new QCheckBox();

    // props
    //res->setMinimumWidth(150);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (!unitText.isEmpty()) {
        QLabel *unitLabel = new QLabel(unitText);
        Lay->addWidget(unitLabel);
        unitLabel->setMinimumWidth(30);
        //unitLabel->setMaximumWidth(30);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QComboBox) + units(QLabel)
QComboBox *addCombo(QString text, QStringList items, QString *unitText,
                    QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QComboBox *res = new QComboBox();

    // add labels
    for (int i = 0; i < items.size(); i++)
    {
        res->addItem(items.at(i));
    }

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    res->setMinimumWidth(0.25*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        Lay->addWidget(unitLabel);
        unitLabel->setMinimumWidth(30);
        unitLabel->setMaximumWidth(30);
    }

    if (ncol>1)
        Lay->addStretch(0);

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QDoubleSpinBox) + units(QLabel)
QDoubleSpinBox *addDoubleSpin(QString text,QString *unitText,
                QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QDoubleSpinBox *res = new QDoubleSpinBox();

    // props
    res->setMinimum(0.);
    res->setKeyboardTracking(0);

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    res->setMinimumWidth(0.25*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        unitLabel->setMinimumWidth(30);
        unitLabel->setMaximumWidth(30);
        Lay->addWidget(unitLabel);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QDoubleSpinBox) + units(QLabel)
QSpinBox *addSpin(QString text, QString *unitText,
                  QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QSpinBox *res = new QSpinBox();

    // props
    res->setKeyboardTracking(0);

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    res->setMinimumWidth(0.25*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        unitLabel->setMinimumWidth(30);
        unitLabel->setMaximumWidth(30);
        Lay->addWidget(unitLabel);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(2);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

void MainWindow::replyFinished(QNetworkReply *pReply)
{
    return;
}
