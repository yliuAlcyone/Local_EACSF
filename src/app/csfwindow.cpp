#pragma GCC diagnostic ignored "-Wshadow"
#include "csfwindow.h"
#include<stdlib.h>
#include <string>
#include <iostream>

#include <QtGui>
#include <QFileDialog>
#include <QBoxLayout>
#include <QTextStream>
#include <QPixmap>


#ifndef Local_EACSF_TITLE
#define Local_EACSF_TITLE "Local_EACSF"
#endif

#ifndef Local_EACSF_CONTRIBUTORS
#define Local_EACSF_CONTRIBUTORS "Tahya Deddah, Martin Styner, Juan Prieto"
#endif

const QString CSFWindow::m_github_url = "https://github.com/NIRALUser/Local_EACSF";

CSFWindow::CSFWindow(QWidget *m_parent)
    :QMainWindow(m_parent)
{
    
    this->setupUi(this);

    batch_processing = false ; 

    model = new CsvTableModel(ModelData, this);
    tableView->setModel(model);
    tableView->setTextElideMode(Qt::ElideLeft);
    tableView->setWordWrap(false);


    QJsonObject root_obj = readConfig(QString(":/config/default_config.json"));
    
    QJsonArray exe_array = root_obj["executables"].toArray();
    QMap<QString, QString> executables;

    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    QBoxLayout* exe_layout = new QBoxLayout(QBoxLayout::LeftToRight, tab_executables);
    m_exeWidget = new ExtExecutablesWidget();

    if (!executables.keys().isEmpty())
    {
        m_exeWidget->buildInterface(executables);
        m_exeWidget->setExeDir(QDir::currentPath());
        exe_layout->addWidget(m_exeWidget,Qt::AlignCenter);
        connect(m_exeWidget, SIGNAL(newExePath(QString,QString)), this, SLOT(updateExecutables(QString,QString)));
    }

    tab->removeTab(3); 
}

CSFWindow::~CSFWindow(){}


QJsonObject CSFWindow::readConfig(QString filename)
{
    QString config_qstr;
    QFile config_qfile;
    config_qfile.setFileName(filename);
    config_qfile.open(QIODevice::ReadOnly | QIODevice::Text);
    config_qstr = config_qfile.readAll();
    config_qfile.close();

    QJsonDocument config_doc = QJsonDocument::fromJson(config_qstr.toUtf8()); 
    QJsonObject root_obj = config_doc.object();
    return root_obj;
}

bool CSFWindow::writeConfig(QString filename)
{
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        std::cout<<"Couldn't open save file."<<std::endl;
        return false;
    }
    QJsonDocument saveDoc(getConfig());
    saveFile.write(saveDoc.toJson());

    std::cout<<"Saved configuration : "<<filename.toStdString()<<endl;
    return true;
}


void CSFWindow::setConfig(QJsonObject root_obj)
{
    QJsonObject data_obj = root_obj["data"].toObject();

    lineEdit_Segmentation->setText(data_obj["Tissu_Segmentation"].toString());
    lineEdit_CSF_Probability_Map->setText(data_obj["CSF_Probability_Map"].toString());
    lineEdit_LH_MID_Surface->setText(data_obj["LH_MID_surface"].toString());
    lineEdit_LH_GM_Surface->setText(data_obj["LH_GM_surface"].toString());
    lineEdit_LH_Inflating_Template->setText(data_obj["LH_Inflating_Template"].toString());
    lineEdit_RH_MID_Surface->setText(data_obj["RH_MID_surface"].toString());
    lineEdit_RH_GM_Surface->setText(data_obj["RH_GM_surface"].toString());
    lineEdit_Left_Atlas_Surface->setText(data_obj["Left_Atlas_Surface"].toString());
    lineEdit_Right_Atlas_Surface->setText(data_obj["Right_Atlas_Surface"].toString());
    lineEdit_RH_Inflating_Template->setText(data_obj["RH_Inflating_Template"].toString());
    lineEdit_Output_Directory->setText(data_obj["Output_Directory"].toString());
    lineEdit_Label->setText(data_obj["Label"].toString());

    QJsonObject param_obj = root_obj["parameters"].toObject();

    lineEdit_Core->setText(param_obj["Slurm_cores"].toString());
    lineEdit_Node->setText(param_obj["Slurm_nodes"].toString());
    lineEdit_Time->setText(param_obj["Slurm_time"].toString());
    lineEdit_Memory->setText(param_obj["Slurm_memory"].toString());

    smooth->setChecked(param_obj["Smooth"].toBool());
    NumberIter_lineEdit->setText(param_obj["Smoothing_numberIter"].toString());
    Bandwith_lineEdit->setText(param_obj["Smoothing_bandwith"].toString());

    clean->setChecked(param_obj["Clean"].toBool());
    radioButton_slurm->setChecked(param_obj["Slurm"].toBool());
    // to test:
    if(param_obj["Slurm"].toBool()==true){radioButton_slurm->setChecked(true);}
    else{radioButton_slurm->setChecked(false);}
    /// /////
    if(param_obj["Slurm"].toBool())
        {slurm_parameters_state(true);}
    else{slurm_parameters_state(false);}

    ClosingRadius->setValue(param_obj["Closing_radius"].toInt());
    DilationRadius->setValue(param_obj["Dilation_radius"].toInt());
    IterationsNumber->setValue(param_obj["Iterations_number"].toInt());
    ImageDimension->setText(param_obj["Image_dimension"].toString());
    InterpolationMargin->setText(param_obj["Interpolation_margin"].toString());

    radioButton_Interpolated->setChecked(param_obj["Interpolated"].toBool());
    radioButton_notInterpolated->setChecked(param_obj["NotInterpolated"].toBool());

    checkBox_75P_Surface->setChecked(param_obj["Use_75P_Surface"].toBool());
    checkBox_Compute_regional_CSF_density->setChecked(param_obj["Compute_regional_CSF_density"].toBool());

    
    QJsonArray exe_array = root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        cout<<exe_obj["path"].toString().toStdString()<<endl;

        this->updateExecutables(exe_obj["name"].toString(), exe_obj["path"].toString());
    }
}

QJsonObject CSFWindow::getConfig(){

    QJsonObject data_obj; 
   
    data_obj["Tissu_Segmentation"]=lineEdit_Segmentation->text();
    data_obj["CSF_Probability_Map"]=lineEdit_CSF_Probability_Map->text();
    data_obj["LH_MID_surface"]=lineEdit_LH_MID_Surface->text();  
    data_obj["LH_Inflating_Template"]=lineEdit_LH_Inflating_Template->text();
    data_obj["RH_MID_surface"]=lineEdit_RH_MID_Surface->text();    
    data_obj["RH_Inflating_Template"]=lineEdit_RH_Inflating_Template->text();  
    data_obj["Output_Directory"]=lineEdit_Output_Directory->text();
    data_obj["Label"]=lineEdit_Label->text();

    QJsonObject param_obj;
   
    if( checkBox_75P_Surface->isChecked())
    {
        data_obj["LH_GM_surface"]=lineEdit_LH_GM_Surface->text();
        data_obj["RH_GM_surface"]=lineEdit_RH_GM_Surface->text();
    }
    if( checkBox_Compute_regional_CSF_density->isChecked())
    {
        data_obj["Left_Atlas_Surface"]=lineEdit_Left_Atlas_Surface->text();
        data_obj["Right_Atlas_Surface"]=lineEdit_Right_Atlas_Surface->text();
    }
    if(radioButton_slurm->isChecked())
    {
        param_obj["Slurm_nodes"] = lineEdit_Node->text();    
        param_obj["Slurm_cores"] = lineEdit_Core->text();
        param_obj["Slurm_time"] = lineEdit_Time->text();
        param_obj["Slurm_memory"] = lineEdit_Memory->text();
    }
    param_obj["Smooth"] = smooth->isChecked();
    if(smooth->isChecked())
    {
        param_obj["Smoothing_numberIter"] = NumberIter_lineEdit->text();    
        param_obj["Smoothing_bandwith"] = Bandwith_lineEdit->text();
    }
    param_obj["Clean"] = clean->isChecked();
    param_obj["Slurm"] = radioButton_slurm->isChecked();

    param_obj["Closing_radius"] = ClosingRadius->value();    
    param_obj["Dilation_radius"] = DilationRadius->value();
    param_obj["Iterations_number"] = IterationsNumber->value();
    param_obj["Image_dimension"] = ImageDimension->text();
    param_obj["Interpolation_margin"] = InterpolationMargin->text();

    param_obj["Interpolated"] = radioButton_Interpolated->isChecked();
    param_obj["NotInterpolated"] = radioButton_notInterpolated->isChecked();

    param_obj["Use_75P_Surface"] = checkBox_75P_Surface->isChecked();
    param_obj["Compute_regional_CSF_density"] = checkBox_Compute_regional_CSF_density->isChecked();

    QJsonObject root_obj;
    root_obj["data"] = data_obj;
    root_obj["parameters"] = param_obj;
    root_obj["executables"] = m_exeWidget->GetExeArray();
    return root_obj;
}

QString CSFWindow::OpenFile(){
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Open File",
                "C://",
                "All files (*.*);; NIfTI File (*.nii *.nii.gz);; NRRD File  (*.nrrd);; Json File (*.json)"
                );
    return filename;
}

QString CSFWindow::SaveFile()
  {
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Document",
        QDir::currentPath(),
        "JSON file (*.json);; Csv file(*.csv)"
        );
    return filename;
  }

QString CSFWindow::OpenDir(){
    QString dirname = QFileDialog::getExistingDirectory(
                this,
                "Open Directory",
                "C://",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );
    return dirname;
}

QString CSFWindow::checkStringValue(QJsonValue str_value)
{
            if(str_value.isUndefined()){
                return QString("");
            }else{
                return str_value.toString();
            }
}

void CSFWindow::infoMsgBox(QString message, QMessageBox::Icon type)
{
    QMessageBox mb;
    mb.setIcon(type);
    mb.setText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}

QList<QMap<QString, QString>*> CSFWindow::readCSV(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream stream(&file);
    QList<QMap<QString, QString>*> modeldata;
    QString separator(","); 
    QStringList keys = stream.readLine().split(separator); //header
    while (stream.atEnd() == false)
    {
        QMap<QString, QString> *line = new QMap<QString, QString>;
        QStringList record = stream.readLine().split(separator);
        for (int i = 0 ; i < keys.size() ; i ++)
        {
            line->insert(keys.at(i), record.at(i));
        }
        modeldata << line;
    }
    file.close();
    return modeldata;
}
bool CSFWindow::writeToCsv(QString filename)
{
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::Append | QIODevice::Text)) {
        std::cout<<"Couldn't open save file."<<std::endl;
        return false;
    }
    QList<QMap<QString, QString>*> modeldata = model->GetModelData();
    QTextStream stream(&saveFile);
    QString separator(",");
    stream << modeldata.at(0)->keys().join(separator) << endl;
    for (int i = 0; i < modeldata.size(); ++i)
    {
        stream << modeldata.at(i)->values().join(separator) << endl;
    }
    stream.flush();
    saveFile.close();
    return true; 
}

//File menu
void CSFWindow::on_actionLoad_Config_File_triggered()
{
        QString filename= OpenFile();
        if (!filename.isEmpty())
        {
            setConfig(readConfig(filename));
        }
}
void CSFWindow::on_actionSave_Config_File_triggered()
{
        QString filename=SaveFile();
        if (!filename.isEmpty())
        {
            if (!filename.endsWith(QString(".json")))
            {
                filename+=QString(".json");
            }
            bool success=writeConfig(filename);
            if (success)
            {
                infoMsgBox(QString("Configuration saved with success : ")+filename,QMessageBox::Information);
            }
            else
            {
                infoMsgBox(QString("Couldn't save configuration file at this location. Try somewhere else."),QMessageBox::Warning);
            }
        }
}
void CSFWindow::on_actionLoad_Csv_File_triggered()
{
    QString filename= OpenFile();
    if (!filename.isEmpty())
    {
        QList<QMap<QString, QString>*> list = readCSV(filename);
        model->SetModelData(list);


        if(!list.isEmpty())
        { 
            QString data_directory = list[0]->value("Output Directory").mid(0, list[0]->value("Output Directory").lastIndexOf("/"));
            QString OutputCSVFile =  data_directory + QString("/Outputs.csv");
            QFile file(OutputCSVFile);
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            file.close();
        } 
    }
}
void CSFWindow::on_actionSave_Csv_File_triggered()
{
    QString filename=SaveFile();
    if (!filename.isEmpty())
    {
        if (!filename.endsWith(QString(".csv")))
        {
            filename+=QString(".csv");
        }
        bool success=writeToCsv(filename);
        if (success)
        {
            infoMsgBox(QString("Csv file saved with success : ")+filename,QMessageBox::Information);
        }
        else
        {
            infoMsgBox(QString("Couldn't save Csv file at this location. Try somewhere else."),QMessageBox::Warning);
        }
    }
}

//Help menu


void CSFWindow::on_actionAbout_triggered()
{
    QString messageBoxTitle = "About " + QString( Local_EACSF_TITLE );
           QString aboutFADTTS;
           aboutFADTTS = "<b>Contributors:</b> " + QString( Local_EACSF_CONTRIBUTORS ) + "<br>"
                  "<b>Github:</b> <a href=" + m_github_url + ">Click here</a><br>";
       QMessageBox::information( this, tr( qPrintable( messageBoxTitle ) ), tr( qPrintable( aboutFADTTS ) ), QMessageBox::Ok );
}

// Window menu

void CSFWindow::on_actionToggle_advanced_mode_toggled(bool toggled)
{
    if (toggled)
    {
        tab->insertTab(3,tab_parameters,QString("Parameters"));
    }
    else
    {
        tab->removeTab(3);
    }
}


// 1st Tab - batch processing

void CSFWindow::on_Data_Directory_clicked()
{
    QString path=OpenDir();
    if (!path.isEmpty())
    {
        lineEdit_Data_Directory->setText(path);
    }
}

void CSFWindow::on_checkBox_75P_Surface_batch_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}
    LH_GM_filter->setEnabled(enab);
    RH_GM_filter->setEnabled(enab);
    lineEdit_LH_GM_filter->setEnabled(enab);
    lineEdit_RH_GM_filter ->setEnabled(enab);
    checkBox_75P_Surface->setChecked(enab);

}
void CSFWindow::on_Find_clicked()
{
   QString data_directory = lineEdit_Data_Directory->text();
   if(data_directory==QString(""))
   {
       QMessageBox::information(
                   this,
                   tr("EACSF"),
                   tr("Please choose a data directory")
               );
   }
   else
   { 
        ModelData.clear(); 
        QDirIterator it(data_directory, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);        
        while(it.hasNext())
        {  
            QString subdirectory = it.next();
            QFileInfo file(subdirectory);
            bool Three_quarters_Surface = checkBox_75P_Surface_batch->isChecked();
            if (file.isDir()) 
            { 
                QDir oDir(file.absoluteFilePath());  
                QMap<QString, QString> *row = new QMap<QString, QString>;
                bool Seg_Found = Find_File(oDir, lineEdit_Seg_filter->text(), QString("Tissu Segmentation"), row);
                bool CSF_Found = Find_File(oDir, lineEdit_CSF_Prob_filter->text(), QString("CSF Probability Map"), row);   
                bool LH_MID_Found = Find_File(oDir, lineEdit_LH_MID_filter->text(), QString("LH MID Surface"), row);
                bool RH_MID_Found = Find_File(oDir, lineEdit_RH_MID_filter->text(), QString("RH MID Surface"), row);
                bool inputs_found = Seg_Found & CSF_Found & LH_MID_Found & RH_MID_Found ;

                if(Three_quarters_Surface)
                {
                    bool LH_GM_Found = Find_File(oDir, lineEdit_LH_GM_filter->text(), QString("LH GM Surface"), row);                
                    bool RH_GM_Found = Find_File(oDir, lineEdit_RH_GM_filter->text(), QString("RH GM Surface"), row);
                    inputs_found = Seg_Found & CSF_Found & LH_MID_Found & RH_MID_Found & LH_GM_Found & RH_GM_Found ;
                }  

                if( inputs_found)
                {
                    row->insert(QString("Output Directory"), file.absoluteFilePath()); 
                    row->insert(QString("Label"), file.fileName());
                    ModelData << row ;  
                }    
            } 
        }
        if(!ModelData.isEmpty())
        { 
            model->SetModelData(ModelData);
            QString OutputCSVFile =  data_directory + QString("/Outputs.csv");
            QFile file(OutputCSVFile);
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            file.close();
        } 
        else{ infoMsgBox(QString("No data found."),QMessageBox::Warning);}            
    }
}

bool CSFWindow::Find_File(QDir oDir, QString filter, QString key, QMap<QString, QString> *vect)
{ 
    QStringList oDirList = oDir.entryList(QDir::Files);
    bool found = false;
    int count = 0;

    for (int i = 0; i < oDirList.size(); ++i)
    {
        QString filename = oDirList.at(i); 
        QRegularExpression re(filter);
        QRegularExpressionMatch match = re.match(QFileInfo( oDir, filename).fileName());
        if (filter != QString("") && match.hasMatch())
        {        
            count = count + 1; 
            found = true;
            auto & map = *vect;
            map.insert(key, QFileInfo( oDir, filename).absoluteFilePath());     
        }
    }
    if (count > 1)
        {
            found = false; 
            infoMsgBox(QString("The Directory :") + oDir.path() + QString(" contains more than one file matching with ") + filter , QMessageBox::Warning);
        }
    return found; 
}

void CSFWindow::on_Add_clicked()
{
    addWidget *add_Widget = new addWidget(0,checkBox_75P_Surface_batch->isChecked());
    connect(add_Widget, SIGNAL(add_to_model( QMap<QString, QString>* )), this, SLOT(addToModel(QMap<QString, QString>* )));
    add_Widget->show();
}

void CSFWindow::addToModel(QMap<QString, QString> *row)
{   
    model->AddRow(row);
}

void CSFWindow::on_Remove_clicked()
{
   QItemSelectionModel *select = tableView->selectionModel();
    if(select->hasSelection()) 
    {
        QModelIndexList indexes = select->selectedRows();
        while (!indexes.isEmpty())
        {
            model->removeRows(indexes.last().row(), 1);
            indexes.removeLast();
        }
    }
    else{ infoMsgBox(QString("No row selcted."),QMessageBox::Warning);}            
}

void CSFWindow::on_Clear_clicked()
{
    model->clear();        
}
void CSFWindow::on_Help_clicked()
{
    QWidget *help = new QWidget();
    help->resize(940, 520);

    QLabel *help_txt_1 = new QLabel(help);
    help_txt_1->setGeometry(20,10,940,30);
    help_txt_1->setText("<b> The Data Directory should be like the following example. </b> <b>If you don't want to use 75P surface no need to LH_GM.vtk and RH_GM.vtk in subjects directories.</b> <b> Each subdirectory ( for example subject_0 ) will be used as the output directory and its name as the label of the corresponding case.</b> " );
    help_txt_1->setWordWrap(true);

    QLabel *Data_directory_image = new QLabel(help);
    Data_directory_image->setGeometry(20,20,800,300);
    QPixmap pic(":/batch/BatchProcessingDirectory.png");
    Data_directory_image->setPixmap(pic);

    QLabel *help_txt_2 = new QLabel(help);
    help_txt_2->setGeometry(20,330,940,30);
    help_txt_2->setText("<b> The CSV File should be like the following example ( the order of the header doesn't matter).</b> <b> Same as above about if you use MID surfaces instead of 75P surfaces no need to LH GM Surface and RH GM Surface in your CSV file: </b>");
    help_txt_2->setWordWrap(true);

    QLabel *CSV_image = new QLabel(help);
    CSV_image->setGeometry(20,250,800,320);
    QPixmap csv_pic(":/batch/CSVFile.png");
    CSV_image->setPixmap(csv_pic);

    QLabel *help_txt_3 = new QLabel(help);
    help_txt_3->setGeometry(20,470,400,30);
    help_txt_3->setText("<b> NB: Regular Expression is used to find the inputs files </b>");

    help->show();
}

void CSFWindow::on_Load_clicked()
{
    QString filename= OpenFile();
    if (!filename.isEmpty())
    {
        QList<QMap<QString, QString>*> list = readCSV(filename);
        model->SetModelData(list);

        if(!list.isEmpty())
        { 
            QString data_directory = list[0]->value("Output Directory").mid(0, list[0]->value("Output Directory").lastIndexOf("/"));
            QString OutputCSVFile =  data_directory + QString("/Outputs.csv");
            QFile file(OutputCSVFile);
            file.open(QIODevice::WriteOnly | QIODevice::Append);
            file.close();
        } 
    }
}

void CSFWindow::on_Export_clicked()
{
    QString filename=SaveFile();
        if (!filename.isEmpty())
        {
            if (!filename.endsWith(QString(".csv")))
            {
                filename+=QString(".csv");
            }
            bool success=writeToCsv(filename);
            if (success)
            {
                infoMsgBox(QString("Csv file saved with success : ")+filename,QMessageBox::Information);
            }
            else
            {
                infoMsgBox(QString("Couldn't save Csv file at this location. Try somewhere else."),QMessageBox::Warning);
            }
        }
}

void CSFWindow::on_batchLocal_clicked(bool checked)
{
    radioButton_local->setChecked(checked);
    slurm_parameters_state(!checked);
}

void CSFWindow::on_batchSlurm_clicked(bool checked)
{
    radioButton_slurm->setChecked(checked);
    slurm_parameters_state(checked); 
}

void CSFWindow::on_Run_Batch_Process_clicked()
{       
    QList<QMap<QString, QString>*> data = model->GetModelData();
    if(!data.isEmpty())
    {
        batch_processing = true ;
        if (batchSlurm->isChecked())
        {  
            for (int row=0;row<model->rowCount();row++)
            {

                lineEdit_Segmentation->setText(data[row]->value("Tissu Segmentation"));
                lineEdit_CSF_Probability_Map->setText(data[row]->value("CSF Probability Map"));
                lineEdit_LH_MID_Surface->setText(data[row]->value("LH MID Surface"));                
                lineEdit_RH_MID_Surface->setText(data[row]->value("RH MID Surface"));                
                lineEdit_Output_Directory->setText(data[row]->value("Output Directory"));
                lineEdit_Label->setText(data[row]->value("Label"));
                if( checkBox_75P_Surface->isChecked())
                {
                    lineEdit_RH_GM_Surface->setText(data[row]->value("RH GM Surface"));
                    lineEdit_LH_GM_Surface->setText(data[row]->value("LH GM Surface"));
                }
                run_Local_EACSF(row);
            }
        }
        if (batchLocal->isChecked())
        {
            int row = 0;
            lineEdit_Segmentation->setText(data[row]->value("Tissu Segmentation"));
            lineEdit_CSF_Probability_Map->setText(data[row]->value("CSF Probability Map"));
            lineEdit_LH_MID_Surface->setText(data[row]->value("LH MID Surface")); 
            lineEdit_RH_MID_Surface->setText(data[row]->value("RH MID Surface"));
            lineEdit_Output_Directory->setText(data[row]->value("Output Directory"));
            lineEdit_Label->setText(data[row]->value("Label"));
            if( checkBox_75P_Surface->isChecked())
            {
                lineEdit_RH_GM_Surface->setText(data[row]->value("RH GM Surface"));
                lineEdit_LH_GM_Surface->setText(data[row]->value("LH GM Surface"));
            }
            run_Local_EACSF(row);
        }       
    }       
}

//2nd tab Inputs

void CSFWindow::on_Segmentation_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_Segmentation->setText(path);
    }
}

void CSFWindow::on_CSF_Probability_Map_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
       lineEdit_CSF_Probability_Map->setText(path);
    }
}

void CSFWindow::on_LH_MID_Surface_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_LH_MID_Surface->setText(path);
    }

}

void CSFWindow::on_LH_GM_Surface_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_LH_GM_Surface->setText(path);
    }
}

void CSFWindow::on_LH_Inflating_Template_clicked()
{

    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_LH_Inflating_Template->setText(path);
    }
}

void CSFWindow::on_RH_MID_Surface_clicked()
{

    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_RH_MID_Surface->setText(path);
    }

}

void CSFWindow::on_RH_GM_Surface_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
       lineEdit_RH_GM_Surface->setText(path);
    }

}

void CSFWindow::on_RH_Inflating_Template_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
       lineEdit_RH_Inflating_Template->setText(path);
    }

}

void CSFWindow::on_output_directory_clicked()
{
   QString path=OpenDir();
    if (!path.isEmpty())
    {
        lineEdit_Output_Directory->setText(path);
    }
}

void CSFWindow::on_checkBox_75P_Surface_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}
    gray_matter_surfaces->setEnabled(enab);
    LH_GM_Surface->setEnabled(enab);
    RH_GM_Surface->setEnabled(enab);
    lineEdit_LH_GM_Surface->setEnabled(enab);
    lineEdit_RH_GM_Surface->setEnabled(enab);
}


// 3rd Tab - Tools path tab
void CSFWindow::updateExecutables(QString exeName, QString path)
{
   m_exeWidget->setExecutable(exeName, path);
}


// 4rd Tab -  Smoothing + Cleaning + Server
void CSFWindow::on_radioButton_slurm_clicked(bool checked)
{
    slurm_parameters_state(checked);
}

void CSFWindow::on_radioButton_local_clicked(bool checked)
{
    slurm_parameters_state(!checked);
}

void CSFWindow::slurm_parameters_state(bool checked)
{
    slurm_parameters->setEnabled(checked);
    Core->setEnabled(checked);
    Memory->setEnabled(checked);
    Node->setEnabled(checked);
    Time->setEnabled(checked);
    lineEdit_Core->setEnabled(checked);
    lineEdit_Memory->setEnabled(checked);
    lineEdit_Node->setEnabled(checked);
    lineEdit_Time->setEnabled(checked);
}

void CSFWindow::on_smooth_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}
    smoothing_parameters_label->setEnabled(enab);
    Smoothing_NumberIter->setEnabled(enab);
    NumberIter_lineEdit->setEnabled(enab);
    Smoothing_Bandwith->setEnabled(enab);
    Bandwith_lineEdit->setEnabled(enab);
}


// 5th tab - Execution

void CSFWindow::prc_finished(QProcess *prc, QString outlog_filename, QString errlog_filename, int row){

    QString exit_message;
    int exitCode =  prc->exitCode();
    QProcess::ExitStatus exitStatus = prc->exitStatus();
   
    exit_message = QString("Local_EACSF pipeline ") + ((exitStatus == QProcess::NormalExit) ? QString("exited with code ") + QString::number(exitCode) : QString("crashed"));
    if (exitCode==0)
    {
        exit_message="<font color=\"green\"><b>"+exit_message+"</b></font>";
        output->append(exit_message);
        cout<<exit_message.toStdString()<<endl;
        QFile file(outlog_filename);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream out(&file); out <<exit_message;
        file.close();
    }
    else
    {
        exit_message="<font color=\"red\"><b>"+exit_message+"</b></font>";
        error->append(exit_message);
        cout<<exit_message.toStdString()<<endl;
        QFile file(errlog_filename);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream out(&file); out << exit_message;
        file.close();
    }  
    if (batch_processing)
    {
       QList<QMap<QString, QString>*> data = model->GetModelData();
        if (row < (model->rowCount()-1))
        {
            lineEdit_Segmentation->setText(data[row + 1]->value("Tissu Segmentation"));
            lineEdit_CSF_Probability_Map->setText(data[row + 1]->value("CSF Probability Map"));
            lineEdit_LH_MID_Surface->setText(data[row + 1]->value("LH MID Surface")); 
            lineEdit_RH_MID_Surface->setText(data[row + 1]->value("RH MID Surface"));
            if( checkBox_75P_Surface->isChecked())
            {
                lineEdit_RH_GM_Surface->setText(data[row + 1]->value("RH GM Surface"));
                lineEdit_LH_GM_Surface->setText(data[row + 1]->value("LH GM Surface"));
            }
            lineEdit_Output_Directory->setText(data[row + 1]->value("Output Directory"));
            lineEdit_Label->setText(data[row + 1]->value("Label"));
            run_Local_EACSF(row+1);           
        }
    }
}

void CSFWindow::CleanFile(QString filename)
{ 
    QFile file(filename);
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    file.close();
}


void CSFWindow::on_Execute_clicked()
{  
    run_Local_EACSF(-1);   //execute the program without batch processing
}

void CSFWindow::run_Local_EACSF(int row)
{
    QProcess *prc; 
    prc = new QProcess;

    QJsonObject root_obj = getConfig();

    CSFScripts csfscripts;
    csfscripts.setConfig(root_obj);
    csfscripts.run_EACSF();

    QJsonObject data_obj = root_obj["data"].toObject();
    QJsonObject param_obj = root_obj["parameters"].toObject();
    QString output_dir = data_obj["Output_Directory"].toString();
    QString label = data_obj["Label"].toString();
    QString scripts_dir = QDir::cleanPath(output_dir + QString("/LocalEACSF") + QString("/PythonScripts"));
    QString outlog_filename = QDir::cleanPath(output_dir + QString("/LocalEACSF") + QString("/output_log.txt"));
    QString errlog_filename = QDir::cleanPath(output_dir + QString("/LocalEACSF") + QString("/errors_log.txt"));
    QString CSF_volume_filename = QDir::cleanPath(output_dir + QString("/LocalEACSF/") + label + QString("_CSFVolume.txt"));
    QFile volume_file(CSF_volume_filename);


    if (radioButton_local->isChecked())
    {  
        
        CleanFile(outlog_filename);
        CleanFile(errlog_filename);

        QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
        QStringList params = QStringList() << main_script;

        prc->setWorkingDirectory(output_dir); 
        prc->setStandardOutputFile(outlog_filename);
        prc->setStandardErrorFile(errlog_filename);

        connect(prc, &QProcess::started, [prc,  this](){
            output->append(QString("Job started successfully"));
        });
        /*connect(prc, &QProcess::errorOccurred, [QProcess::ProcessError error, this](){
            error->append(error);
        });*/
        connect(prc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [prc, outlog_filename, errlog_filename, row, this ](){
            prc_finished(prc, outlog_filename, errlog_filename, row);  
        });
       
        QMap<QString, QString> executables = m_exeWidget->GetExeMap();
        prc->start(executables[QString("python3")], params);                 
    }
    
    if (radioButton_slurm->isChecked())
    {
        
        if(volume_file.exists()) 
        {       
            output->append(QString("Compute Local EACSF Density already done for : ") + data_obj["Label"].toString() );
        } 
        else
        {
            QString slurm_script = QDir::cleanPath(scripts_dir + QString("/slurm-job"));
            QString time = QString("--time=") + param_obj["Slurm_time"].toString();
            QString memory = QString("--mem=") + param_obj["Slurm_memory"].toString();
            QString core = QString("--ntasks=") + param_obj["Slurm_cores"].toString();
            QString node = QString("--nodes=") + param_obj["Slurm_nodes"].toString();
            QString output_file = QString("--output=") + outlog_filename ;
            QString error_file = QString("--error=") + errlog_filename ;

            QStringList params = QStringList() << time << memory << core << node << output_file << error_file << slurm_script;
            prc->setWorkingDirectory(output_dir);
            connect(prc, &QProcess::readyReadStandardOutput, [prc, outlog_filename, this](){
                QString output_log(prc->readAllStandardOutput()); 
                output->append(output_log);
            });
            connect(prc, &QProcess::readyReadStandardError, [prc, errlog_filename, this](){
                QString error_log(prc->readAllStandardError());
                error->append(error_log);
            });
            prc->start(QString("sbatch"), params);
        }       
    }  
}
//6th QC 

void CSFWindow::on_output_path_clicked()
{
    QString path=OpenDir();

    if (!path.isEmpty())
    {
        lineEdit_output_path->setText(path);

        QString jsonfile = lineEdit_output_path->text() + QString("/LocalEACSF") + QString("/Local_EACSF_config.json");
        QJsonObject root_obj = readConfig(jsonfile);
        QJsonObject data_obj = root_obj["data"].toObject();
        QString Label = data_obj["Label"].toString();
        QString inflated_mid = lineEdit_output_path->text() + QString("/LocalEACSF") + QString("/LH_Directory") + QString("/") + Label + QString("_LH_MID_Inflated.vtk");
        QString inflated_75p = lineEdit_output_path->text() + QString("/LocalEACSF") + QString("/LH_Directory") + QString("/") + Label + QString("_LH_75P_Inflated.vtk");
        QFileInfo check_file1(inflated_mid);
        QFileInfo check_file2(inflated_75p);
        if(check_file1.exists() || check_file2.exists()){VisualiseCSFDensityinflating->setEnabled(true);}
    }      
}
void CSFWindow::on_CSVFile_path_clicked()
{
    QString path=OpenFile();

    if (!path.isEmpty())
    {
       lineEdit_CSVFile->setText(path);
    }
}

void CSFWindow::on_visualize_clicked()
{
    QProcess *visualization;
    visualization = new QProcess;
    QString OutputDirectory =lineEdit_output_path->text();
    QString LH_Directory = QDir::cleanPath(OutputDirectory + QString("/LocalEACSF") + QString("/LH_Directory"));
    QString RH_Directory = QDir::cleanPath(OutputDirectory + QString("/LocalEACSF") + QString("/RH_Directory"));
    

    QString jsonfile = OutputDirectory + QString("/LocalEACSF") + QString("/Local_EACSF_config.json");
    QJsonObject root_obj = readConfig(jsonfile);
    QJsonObject data_obj = root_obj["data"].toObject();
    QString Label = data_obj["Label"].toString();
    qDebug() << Label ; 


    
    if (VisualiseCSFDensity->isChecked())
    {

        QString LH_CSFDensity = LH_Directory + QString("/") + Label + QString("_LH_MID_CSF_Density.vtk");
        QString RH_CSFDensity  = RH_Directory + QString("/") + Label + QString("_RH_MID_CSF_Density.vtk");
        QFileInfo check_file(LH_CSFDensity);
        if(!check_file.exists())
        {
            LH_CSFDensity = LH_Directory + QString("/") + Label + QString("_LH_75P_CSF_Density.vtk");
            RH_CSFDensity  = RH_Directory + QString("/") + Label + QString("_RH_75P_CSF_Density.vtk");
        }
        QStringList arguments = QStringList() << QString("-v") << LH_CSFDensity  << QString("-v")<< RH_CSFDensity ;
        visualization->setWorkingDirectory(OutputDirectory);
        visualization->start(QString("ShapePopulationViewer"), arguments);
    }

    if (VisualiseCSFDensityinflating->isChecked())
    {
        QString LH_CSFDensity = LH_Directory + QString("/") + Label + QString("_LH_MID_Inflated.vtk");
        QString RH_CSFDensity  = RH_Directory + QString("/") + Label +  QString("_RH_MID_Inflated.vtk");
        QFileInfo check_file(LH_CSFDensity);
        if(!check_file.exists())
        {
            LH_CSFDensity = LH_Directory + QString("/") + Label +  QString("_LH_75P_Inflated.vtk");
            RH_CSFDensity  = RH_Directory + QString("/") + Label +  QString("_RH_75P_Inflated.vtk");
        }
        QStringList arguments = QStringList() << QString("-v") << LH_CSFDensity  << QString("-v")<< RH_CSFDensity ;
        visualization->setWorkingDirectory(OutputDirectory);
        visualization->start(QString("ShapePopulationViewer"), arguments);
    }
    if (VisualiseRightVisitation->isChecked())
    {

        QString CSF_Probability_Map = RH_Directory + QString("/") + Label + QString("_CSF_Probability_Map.nrrd");
        QString RH_visitation_map = RH_Directory + QString("/") + Label + QString("_RH_Visitation.nrrd");
        QStringList arguments = QStringList()<< CSF_Probability_Map  << QString("-o") << RH_visitation_map  ;
        visualization->setWorkingDirectory(OutputDirectory);
        visualization->start(QString("MriWatcher"), arguments);
    }
    if (VisualiseLeftVisitation->isChecked())
    {
        QString CSF_Probability_Map = LH_Directory + QString("/") + Label + QString("_CSF_Probability_Map.nrrd");
        QString LH_visitation_map = LH_Directory + QString("/") + Label + QString("_LH_Visitation.nrrd");
        QStringList arguments = QStringList()<< CSF_Probability_Map  << QString("-o") << LH_visitation_map  ;
        visualization->setWorkingDirectory(OutputDirectory);
        visualization->start(QString("MriWatcher"), arguments);
    }    
}

void CSFWindow::on_Compare_clicked()
{
    QString OutputDirectory =lineEdit_output_path->text();
    QString jsonfile = lineEdit_output_path->text() + QString("/LocalEACSF") + QString("/Local_EACSF_config.json");
    QJsonObject root_obj = readConfig(jsonfile);
    QJsonObject data_obj = root_obj["data"].toObject();
    QString Label = data_obj["Label"].toString();
    QString CSFVolume = QDir::cleanPath(OutputDirectory + QString("/LocalEACSF") +  QString("/") + Label  + QString("_CSFVolume.txt"));   
    qDebug() << CSFVolume;
    QFile file(CSFVolume);    
    if(!file.exists()) 
    {infoMsgBox(QString(" No volume file found") , QMessageBox::Warning);} 
    else
    {
        vector< vector <int> > tableau{{0,0},{1,0},{0,1},{1,1}};
        int i =0;
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        while (!file.atEnd())
        {            
            QString line = file.readLine();
            QStringList splitline = line.split(QString(" = "));
            QString result = splitline.at(1);  
            tableWidget->setItem( tableau[i][0] ,tableau[i][1], new QTableWidgetItem( result.remove(QChar('\n'), Qt::CaseInsensitive) ));
            i=i+1;
        } 
        for(int row =0; row <2; row++)
        {
            QString left = tableWidget->item(row,0)->text();
            QString right = tableWidget->item(row,1)->text();
            float difference = (left.toFloat() - right.toFloat())/ ((left.toFloat() + right.toFloat())/2);
            tableWidget->setItem( row, 2, new QTableWidgetItem( QString::number( difference * 100, 'f' , 2 ))); 
        }                   
    }
}

void CSFWindow::on_visualize_batch_clicked()
{
    QProcess *visualization_batch;
    visualization_batch = new QProcess;
    QString CSVFile =lineEdit_CSVFile->text();
    QList<QMap<QString, QString>*> data = readCSV(CSVFile);
    QStringList arguments = QStringList();

    if (VisualiseCSFDensityBatch->isChecked())
    {
        QStringList arguments = QStringList() << QString("--csv") << CSVFile;
        visualization_batch->start(QString("ShapePopulationViewer"), arguments);
    }
    if (VisualiseVisitationMapLeftBatch->isChecked())
    {
        for (int i = 0; i < data.size(); ++i)
        {
               arguments << data.at(i)->value(QString("LH Visitation Map"));
        }
        visualization_batch->start(QString("MriWatcher"), arguments);
    }
    if (VisualiseVisitationMapRightBatch->isChecked())
    {
        for (int i = 0; i < data.size(); ++i)
        {
            arguments << data.at(i)->value(QString("RH Visitation Map"));
        }
        visualization_batch->start(QString("MriWatcher"), arguments);
    }
}

void CSFWindow::on_help_clicked()
{
    QWidget *help = new QWidget();
    help->resize(900, 200);

    QLabel *help_txt_1 = new QLabel(help);
    help_txt_1->setGeometry(20,10,800,30);
    help_txt_1->setText("<b> The CSV file  should be like the following example. this CSV file is created while batch processing and you can find it in the data directory provide as input in batch processing and its called Outputs.csv : </b>");
    help_txt_1->setWordWrap(true);

    QLabel *Data_directory_image = new QLabel(help);
    Data_directory_image->setGeometry(20,30,800,150);
    QPixmap pic(":/batch/BatchCSVFile.png");
    Data_directory_image->setPixmap(pic);

    help->show();
}

void CSFWindow::on_Left_Atlas_Surface_clicked()
{
    QString path=OpenFile();

       if (!path.isEmpty())
       {
           lineEdit_Left_Atlas_Surface->setText(path);
       }

}

void CSFWindow::on_Right_Atlas_Surface_clicked()
{QString path=OpenFile();

    if (!path.isEmpty())
    {
        lineEdit_Right_Atlas_Surface->setText(path);
    }


}

void CSFWindow::on_checkBox_Compute_regional_CSF_density_stateChanged(int state)
{
    bool enab;
    if (state==Qt::Checked){enab=true;}
    else{enab=false;}

    Left_Atlas_Surface->setEnabled(enab);
    Right_Atlas_Surface->setEnabled(enab);
    lineEdit_Left_Atlas_Surface->setEnabled(enab);
    lineEdit_Right_Atlas_Surface->setEnabled(enab);

}
