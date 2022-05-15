#include "IsoViewer.h"

/********** CONSTRUCTOR **********/

IsoViewer::IsoViewer(QWidget *parent)
    : QMainWindow(parent)
{
    this->ui.setupUi(this);

    // Sets an icon for the window
    this->setWindowIcon(QIcon("favicon.png"));

    // Set the default path in case the config file has an empty one
    this->folder = "C:/helios1/archivio";

    this->config = new Config(this->config_file);

    this->coord_manager = new CoordManager();

    /* 
        Format systems language to use it as default, 
        the initOptions() function will change it if a valid config file is found.
    */
    this->language = QLocale::system().name(); // e.g. "it_IT" then truncated to "it"
    this->language.truncate(this->language.lastIndexOf('_')); // e.g. "it"
    this->loadLanguage(this->language);

    // Set the options according to the config file
    this->initOptions();

    this->createLanguageMenu();

    // Instantiates a scene
    this->scene = new MyGraphicScene();
    // Intercepts the signal emitted and this->connects it to the mousePosition() method
    this->connect(this->scene, &MyGraphicScene::signalMousePos, this, &IsoViewer::mousePosition);

    // Assign the scene to the canvas
    this->ui.canvas->setScene(scene);
    // Set the canvas alignment
    this->ui.canvas->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // Bind the buttons to the corresponding method to fire
    this->connect(this->ui.btn_draw,        &QPushButton::released, this, &IsoViewer::draw);
    this->connect(this->ui.btn_reset,       &QPushButton::released, this, &IsoViewer::fullReset);
    this->connect(this->ui.btn_browse_file, &QPushButton::released, this, &IsoViewer::browseFile);

    // Bind the checkboxes to the corresponding method to persist the selection
    this->connect(this->ui.chk_fit,         &QCheckBox::stateChanged, this, &IsoViewer::toggleFit);
    this->connect(this->ui.chk_autoresize,  &QCheckBox::stateChanged, this, &IsoViewer::toggleAutoresize);
    this->connect(this->ui.chk_color,       &QCheckBox::stateChanged, this, &IsoViewer::toggleColor);
    this->connect(this->ui.chk_gradient,    &QCheckBox::stateChanged, this, &IsoViewer::toggleGradient);
    this->connect(this->ui.chk_zmax,        &QCheckBox::stateChanged, this, &IsoViewer::toggleZMax);

    // Input fieldsand labels styling
    this->ui.lbl_x_min_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_x_max_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_y_min_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_y_max_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_z_max_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_rectangle_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_working_time_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_mouse_pos_x->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_mouse_pos_y->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_offset_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_eng_dst_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");
    this->ui.lbl_pos_dst_value->setStyleSheet("background-color: #DDDDDD; border: 1px solid #BBBBBB;");

    // Timer to manage the delay when regenerating the drawing when the scene is resized
    this->reset_timer = new QTimer();
    
    /* 
        Set the timer as single shot.
        I'm not directly using a singleshot timer because it doesn't have the stop() method which
        is required to simulate a reset as implemented in the resizeEvent() method
    */
    this->reset_timer->setSingleShot(true);

    // Connect the timer's timeout signal to the draw() method
    this->connect(reset_timer, &QTimer::timeout, this, &IsoViewer::draw);
    
    // Says whether the timer must be used or not
    this->delay_enabled = true;
    
    // Delay in milliseconds;
    this->delay_timeout = 500;
}


/********** DESTRUCTOR **********/

IsoViewer::~IsoViewer()
{
    delete this->scene;
    delete this->reset_timer;
    delete this->config;
    delete this->coord_manager;
}


/********** OVERRIDES **********/

void IsoViewer::resizeEvent(QResizeEvent* event)
{
    // When the main window is resized, the scene size must be adjusted

    QVector2D canvas_size = getCanvasSize();
    this->scene_w = canvas_size.x();
    this->scene_h = canvas_size.y();

    // If there is something on the scene and the auto-resize is enabled
    if (this->iso_drawn && this->ui.chk_autoresize->isChecked())
    {
        // If the timer is enabled
        if (this->delay_enabled)
        {
            // If the timer has been started
            if (this->resize_timer_running)
            {
                // Stop the timer, because the user is still resizing the window
                this->reset_timer->stop();
                // Says that the timer is no more active
                this->resize_timer_running = false;
            }

            // Start the timer
            this->reset_timer->start(this->delay_timeout);
            // Says that the timer is active
            this->resize_timer_running = true;
        }
        // If the timer is not enabled
        else
        {
            // Regenerate the drawing
            this->draw();
        }
    }

    // Adjust the eventual file list to the label where they are shown
    if (this->selected_files_full_string != "")
    {
        Helpers::elideText(*this->ui.lbl_selected_file, this->selected_files_full_string);
    }
}

void IsoViewer::showEvent(QShowEvent* event)
{
    // Initialize the scene's size to also have a valid position for the mouse pointer
    QVector2D canvas_size = getCanvasSize();
    this->scene_w = canvas_size.x();
    this->scene_h = canvas_size.y();
}

/********** PUBLIC FUNCTIONS **********/


/********** INIT FUNCTIONS **********/

void IsoViewer::initOptions()
{
    // If the config file does not exist or it is not valid
    if (!Helpers::pathFileExists(this->config_file) || !this->config->checkConfigFile())
    {
        // Initialize a new config file with the default values
        this->config->initConfigFile(this->language.toStdString());

        // Now a new valid config file exists, call again this function to load the updated file
        this->initOptions();
    }
    else
    {
        // Open the config file in read mode
        std::ifstream ifs(this->config_file);

        // Instantiate a json object
        json j;

        // Import the file content into the json object
        ifs >> j;

        ifs.close();

        // Set the language
        std::string lang = j["lang"];
        this->language = QString::fromUtf8(lang.c_str());
        this->loadLanguage(this->language);

        // If the folder key is present
        if (j["folder"] != "")
            // Set it as the current folder to open
            this->folder = j["folder"];

        // Set the checkboxes according to the config file content

        if(j["fit"] == "0")
            this->ui.chk_fit->setChecked(false);
        else
            this->ui.chk_fit->setChecked(true);

        if (j["autoresize"] == "0")
            this->ui.chk_autoresize->setChecked(false);
        else
            this->ui.chk_autoresize->setChecked(true);

        if (j["color"] == "0")
            this->ui.chk_color->setChecked(false);
        else
            this->ui.chk_color->setChecked(true);

        if (j["gradient"] == "0")
            this->ui.chk_gradient->setChecked(false);
        else
            this->ui.chk_gradient->setChecked(true);

        if (j["zmax"] == "0")
            this->ui.chk_zmax->setChecked(false);
        else
            this->ui.chk_zmax->setChecked(true);
    }
}


/********** UPDATE FUNCTIONS **********/

void IsoViewer::updateOptions(const std::string& key, const std::string& value)
{
    bool reload = false;

    /* 
        If the config file does not exists or it is not valid, 
        (this shouldn't happen since it is checked as the software runs,
        but a user playing the joker could delete it while the software is running).
     */ 
    if (!Helpers::pathFileExists(this->config_file) || !this->config->checkConfigFile())
    {
        // Initialize a new one
        this->config->initConfigFile(this->language.toStdString());

        // A new config file has been generated
        reload = true;
    }

    this->config->updateConfigFile(key, value);

    if (reload)
        // Call the initOptions() function to load the updated file
        this->initOptions();
}

void IsoViewer::toggleFit()
{
    if (this->ui.chk_fit->isChecked())
        this->updateOptions("fit", "1");
    else
        this->updateOptions("fit", "0");
}

void IsoViewer::toggleAutoresize()
{
    if (this->ui.chk_autoresize->isChecked())
        this->updateOptions("autoresize", "1");
    else
        this->updateOptions("autoresize", "0");
}

void IsoViewer::toggleColor()
{
    if (this->ui.chk_color->isChecked())
        this->updateOptions("color", "1");
    else
        this->updateOptions("color", "0");
}

void IsoViewer::toggleGradient()
{
    if (this->ui.chk_gradient->isChecked())
        this->updateOptions("gradient", "1");
    else
        this->updateOptions("gradient", "0");
}

void IsoViewer::toggleZMax()
{
    if (this->ui.chk_zmax->isChecked())
        this->updateOptions("zmax", "1");
    else
        this->updateOptions("zmax", "0");
}

void IsoViewer::updateFolder(const QString& f)
{
    /* 
        f variable is a QString, convert it to std::string before to assign it to the class member 
        and before to call the update function
    */
    this->folder = f.toStdString();

    this->updateOptions("folder", this->folder);
}

void IsoViewer::updateLanguage(const QString& l)
{
    /*
        In this case, the class member "language" is a QString itself, no need to convert l
        to stdString before the assignment
    */
    this->language = l;

    /* 
        The convertion must be done here because the options are stored inside the config file
        as stdStrings
    */
    this->updateOptions("lang", this->language.toStdString());
}

/********** ACCESSORS **********/

void IsoViewer::mousePosition(const QPointF& pos)
{
    // Contains the numbers to be print into the labels as string
    QString text_number;

    this->ui.lbl_mouse_pos_x->setText(text_number.asprintf("%.1f", pos.x() * (1 / this->scale_factor)));
    // Scene height - pos y because, on the scene, y grows downwards, while we want to see it growing upwards
    this->ui.lbl_mouse_pos_y->setText(text_number.asprintf("%.1f", (this->scene_h - pos.y()) * (1 / this->scale_factor)));
}

QVector2D IsoViewer::getCanvasSize()
{
    return QVector2D(
        static_cast<float>(ui.canvas->width() - this->canvas_expanded),
        static_cast<float>(ui.canvas->height() - this->canvas_expanded)
    );
}


/********** MODIFIERS **********/


/********** RESETTERS **********/

void IsoViewer::fullReset()
{
    this->resetErrors();
    this->resetScene();
    this->ui.in_width->setText("0");
    this->ui.in_height->setText("0");
    this->ui.in_tool_speed->setText("1000");
    this->ui.chk_sculpture->setChecked(false);
}

void IsoViewer::resetErrors()
{
    this->ui.in_width->setStyleSheet("border: 1px solid black");
    this->ui.in_height->setStyleSheet("border: 1px solid black");
    this->ui.in_tool_speed->setStyleSheet("border: 1px solid black");
    this->ui.lbl_selected_file->setStyleSheet("border: none");
    this->ui.statusbar->showMessage("");
}

void IsoViewer::resetScene(bool reset_to_draw)
{
    /* 
        Remove the scene from the view
        this is not really required, but it speeds up the reset process
    */
    this->ui.canvas->setScene(NULL);
    // Reset the scene
    this->scene->clear();
    this->scale_factor = 1;
    this->ui.lbl_x_min_value->setText("");
    this->ui.lbl_x_max_value->setText("");
    this->ui.lbl_y_min_value->setText("");
    this->ui.lbl_y_max_value->setText("");
    this->ui.lbl_z_max_value->setText("");
    this->ui.lbl_eng_dst_value->setText("");
    this->ui.lbl_pos_dst_value->setText("");
    this->ui.lbl_rectangle_value->setText("");
    this->ui.lbl_offset_value->setText("");
    this->ui.lbl_working_time_value->setText("");

    if (!reset_to_draw)
    {
        this->iso_files.clear();
        this->selected_files_full_string = "";
        this->ui.lbl_selected_file->setText("");
    }

    // Says that no drowing is on the scene
    this->iso_drawn = false;

    // Reassign the scene to the canvas
    this->ui.canvas->setScene(this->scene);
}

void IsoViewer::cancelDrawing()
{
    this->resetScene();
    coord_manager->resetCoordinatesLimits();
}


/********** OTHERS **********/

void IsoViewer::browseFile()
{
    // Filter to show only PGR files
    QString filter = "pgr(*.PGR)";

    QString file_folder;
    QString file_name;

    // Allows multiple files selection
    this->iso_files = QFileDialog::getOpenFileNames(this, tr("Apri File"), QString::fromUtf8(this->folder.c_str()), filter);

    // Builds the string containing all the names of the selected files
    foreach(QString file, this->iso_files)
    {
        QFileInfo fi(file);
        file_folder = fi.absolutePath();
        file_name = fi.fileName();
        this->selected_files_full_string += "\"" + file_name + "\" ";
    }

    // Elides, if necessary, the string to fit the selected files label and prints it
    QString elided_text = Helpers::elideText(*this->ui.lbl_selected_file, this->selected_files_full_string);

    // Print the elided text into the label
    this->ui.lbl_selected_file->setText(elided_text);

    // Update the last used folder if the string is not empty (if the user cancels the operation)
    if (file_folder != "")
    {
        this->updateFolder(file_folder);
    }
}

bool IsoViewer::checkData()
{
    // Used to know if the string to int convertion succeeds
    bool ok = true;

    // For each selected file
    foreach(QString f, this->iso_files)
    {
        QFileInfo fi(f);
        QFile file(f);
        // Verify that the file exists and has the proper extension
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text) || fi.suffix() != "PGR")
        {
            this->ui.lbl_selected_file->setStyleSheet("border: 1px solid red");
            this->ui.statusbar->showMessage(tr("Selezionare solo file ISO validi"));
            return false;
        }
        // If the file exists and it has the PGR extension
        else
        {
            QTextStream in(&file);
            QString line = in.readLine();

            // If the file does not begin with the string "QUOTE RELATIVE"
            if (line.trimmed() != "QUOTE RELATIVE")
            {
                this->ui.lbl_selected_file->setStyleSheet("border: 1px solid red");

                this->ui.statusbar->showMessage(tr("Selezionare solo file ISO validi"));
                return false;
            }
        }
    }

    // If the width field is empty
    if (this->ui.in_width->text() == "")
    {
        // Set to 0
        this->ui.in_width->setText("0");
    }

    /* 
        Check if the width value is valid:
        try to convert it to integer, not float, because if this succeeds, it will be possible to
        directly use the number, the width, being considered in mm, has to be an integer.
    */
    int width = this->ui.in_width->text().toInt(&ok);
    if (!ok || width < 0)
    {
        this->ui.in_width->setStyleSheet("border: 1px solid red");
        this->ui.statusbar->showMessage(tr("Indicare una larghezza valida (numero intero positivo o 0)"));
        return false;
    }
    // The value is valid
    else
    {
        // But it could be a decimal number, get only the integer part
        this->ui.in_width->setText(QString::number(width));
    }

    // If the height field is empty
    if (this->ui.in_height->text() == "")
    {
        // Set to 0
        this->ui.in_height->setText("0");
    }

    /* 
        Check if the height value is valid
        Same as the width case
    */
    ok = true;
    int height = this->ui.in_height->text().toInt(&ok);
    if (!ok || height < 0)
    {
        this->ui.in_height->setStyleSheet("border: 1px solid red");
        this->ui.statusbar->showMessage(tr("Indicare un'altezza valida (numero intero positivo o 0)"));
        return false;
    }
    // The value is valid
    else
    {
        // But it could be a decimal number, get only the integer part
        this->ui.in_height->setText(QString::number(height));
    }

    // If the speed tool value field is empty
    if (this->ui.in_tool_speed->text() == "")
    {
        // Set to 1000
        this->ui.in_tool_speed->setText("1000");
    }

    /*
        Check if the speed value is valid
        Same as the width case
    */
    ok = true;
    int tool_speed = this->ui.in_tool_speed->text().toInt(&ok);
    if (!ok || tool_speed <= 0)
    {
        this->ui.in_tool_speed->setStyleSheet("border: 1px solid red");
        this->ui.statusbar->showMessage(tr("Indicare una velocità valida (numero intero positivo)"));
        return false;
    }
    // The value is valid
    else
    {
        // But it could be a decimal number, get only the integer part
        this->ui.in_tool_speed->setText(QString::number(tool_speed));
    }

    // Check if only the width has been set
    if (width > 0 && height == 0)
    {
        // Both the sizes must be set or none of them
        this->ui.in_height->setStyleSheet("border: 1px solid red");
        this->ui.statusbar->showMessage(tr("Indicare entrambe le dimensioni o nessuna"));
        return false;
    }

    // Check if only the height has been set
    if (height > 0 && width == 0)
    {
        // Both the sizes must be set or none of them
        this->ui.in_width->setStyleSheet("border: 1px solid red");
        this->ui.statusbar->showMessage(tr("Indicare entrambe le dimensioni o nessuna"));
        return false;
    }

    return true;
}

float IsoViewer::scaleFactor(int w, int h)
{
    /* 
        I do not use this->scene_w and this->scene_h set into showEvent()
        for the size of the scene, because after that the window is shown it could have been
        resized and the canvas size could have been changed.
    */

    // Canvas size
    QVector2D canvas_size = this->getCanvasSize();

    // Calculates the scale factor to fit the width
    float scale_x = canvas_size.x() / w;
    // Calculates the scale factor to fit the hight
    float scale_y = canvas_size.y() / h;

    // Return the smallest scale factor
    if (scale_x <= scale_y)
        return scale_x;
    else
        return scale_y;
}

void IsoViewer::setScene()
{
    // Reset the scene
    this->resetScene(true);

    // Size set for the slab
    int width = this->ui.in_width->text().toInt();
    int height = this->ui.in_height->text().toInt();

    // Canvas size
    QVector2D canvas_size = this->getCanvasSize();

    // If the drawing must be adapted to the canvas size
    if (this->ui.chk_fit->isChecked())
    {
        // If a width has been set, also the height is set, the validate() method takes care of that
        if (width > 0)
        {
            // Calculate the scale factor to fit the scene
            this->scale_factor = this->scaleFactor(width, height);

            // Adjust the slab size by the scale factor (also used ahead)
            width = width * this->scale_factor;
            height = height * this->scale_factor;

            // Assign the adjusted slab size to the scene
            this->scene_w = width;
            this->scene_h = height;
        }
        // The slab size has not been set
        else
        {
            // To calculate the scale factor, consider the drwaing size
            this->scale_factor = this->scaleFactor(coord_manager->getXMax(), coord_manager->getYMax());

            // Assign the canvas size
            this->scene_w = canvas_size.x();
            this->scene_h = canvas_size.y();
        }
    }
    else
    {
        // If a width has been set, also the hight is set, the validate() method takes care of that
        if (width > 0)
        {
            this->scale_factor = this->scaleFactor(width, height);

            /*
                The scale factor must be != 1 only if the regular drawing's size exceeds the canvas
                which happens if the scale factor calculated is less than 1.
            */
            if (this->scale_factor >= 1)
            {
                // No need to fit the drawing, reset the scale factor to 1
                this->scale_factor = 1;

                /* 
                    Both the canvas' dimensions are bigger or same then the relative ones of the drawing
                    Set the scene as big as the canvas.
                */
                this->scene_w = canvas_size.x();
                this->scene_h = canvas_size.y();
            }
            // The scale factor is less than 1, meaning that at least one drawing dimension exceedes the canvas
            else
            {
                // Adjust the slab size by the scale factor (also used ahead)
                width = width * this->scale_factor;
                height = height * this->scale_factor;

                // Assign the adjusted slab size to the scene
                this->scene_w = width;
                this->scene_h = height;
            }
        }
        // The slab size has not been set
        else
        {
            // To calculate the scale factor, consider the drwaing size
            this->scale_factor = this->scaleFactor(coord_manager->getXMax(), coord_manager->getYMax());

            // As above
            if (this->scale_factor > 1)
            {
                this->scale_factor = 1;
            }

            // Assign the canvas size
            this->scene_w = canvas_size.x();
            this->scene_h = canvas_size.y();
        }
    }

    // Set the scene as big as the canvas
    this->scene->setSceneRect(0, 0, this->scene_w, this->scene_h);

    // If the size for the slab has been set
    if (width > 0)
    {
        // Draw a rectangle defining it
        this->scene->addRect(QRectF(0, this->scene_h, width, -height));
    }
}

int IsoViewer::workingTime(const float& eng_dst, const float& pos_dst)
{
    // Total distance in millimiters
    int tot_dst = eng_dst + pos_dst;

    // Seconds to complete the job(distance is in mm, speed in mm/min)
    int seconds = (tot_dst / this->ui.in_tool_speed->text().toFloat()) * 60;

    return seconds;
}

int IsoViewer::mapRange(const float& value, const float& source_min, const float& source_max, const float& target_min, const float& target_max)
{
    // Figure out how 'wide' each range is
    float left_span = source_max - source_min;
    float right_span = target_max - target_min;

    // Convert the left range into a 0 - 1 range (float)
    float value_scaled = (value - source_min) / left_span;
    // Convert the 0 - 1 range into a value in the right range.
    int ranged_value = int(target_min + (value_scaled * right_span));

    if (ranged_value < target_min)
    {
        ranged_value = target_min;
    }
    else if (ranged_value > target_max)
    {
        ranged_value = target_max;
    }

    return ranged_value;
}

void IsoViewer::draw()
{
    this->resetErrors();

    // If all the inputs are correct
    if (this->checkData())
    {
        /*************************************************************
         *  VARIABLES INITIALIZATION
         *************************************************************/

         // These keep track of the distance covered by the tool
        float engraving_dst = 0.f;
        float positioning_dst = 0.f;

        // X, Y and Z distance for each movement
        float dx = 0.f;
        float dy = 0.f;
        float dz = 0.f;

        // Vertices defining the current movement segment
        QPoint p1, p2;

        // Current position of the tool, initialized over the origin
        QVector3D current_position = QVector3D(0, 0, 0);
        // Number of segments drawn
        int lines = 0;

        // Says if the tool is currently lowering
        bool lowering = false;
        // Says if the tool is engraving
        bool drawing = false;
        // Says if a repositioning is required
        bool repositioning = false;

        // Start and end colors to draw the segments with a gradient effect
        int start_mapped_color = 0;
        int end_mapped_color = 0;

        // Colors to use to draw the segments and for monochromatic drawing
        QColor start_color(0, 0, 0), end_color(0, 0, 0), color(0, 0, 0);

        // Gradient to use to draw the segments
        QLinearGradient gradient;
        // Pen to draw the segments
        QPen pen;

        // Contains the numbers to be print into the labels as string
        QString text_number;

        /*************************************************************/

        // Reset the minand max values
        coord_manager->resetCoordinatesLimits();

        // List of coordinates extracted by the iso files
        QList<QVector3D> coords = coord_manager->processCoordinates(
            this->iso_files, 
            this->ui.chk_sculpture->isChecked(),
            this->ui.chk_fit->isChecked(),
            this->ui.in_width->text().toInt() != 0
        );

        if (coords.isEmpty())
        {
            this->cancelDrawing();
            return;
        }

        int num_coords = coords.length();

        // Prepare the scene
        this->setScene();

        // Absolute z_max value used to calculate the color of the segments
        float abs_z_max = abs(coord_manager->getZMax());

        bool draw_color = this->ui.chk_color->isChecked();
        /* 
            Use the gradient effect if the user required it explicitly, or if the iso file contains a sculpture.
            The content of a sculpture iso file is not recognizable without the gradient effect.
        */
        bool draw_gradient = this->ui.chk_gradient->isChecked() || this->ui.chk_sculpture->isChecked();

        /* 
            If the user doesn't want to fit the visible area AND the scale factor is > 1
            The second condition is to force the scaling down when the drawing size exceeds the canvas size
            in that case the scale factor will be < 1 and must not be changed or part of it will not be visible.
        */
        if (!this->ui.chk_fit->isChecked() && this->scale_factor > 1)
        {
            // Set the scale factor to a neutral value
            this->scale_factor = 1;
        }

        // Limit the values to the 3 decimals and print them on the proper label
        this->ui.lbl_x_min_value->setText(text_number.asprintf("%.3f", coord_manager->getXMin()));
        this->ui.lbl_x_max_value->setText(text_number.asprintf("%.3f", coord_manager->getXMax()));
        this->ui.lbl_y_min_value->setText(text_number.asprintf("%.3f", coord_manager->getYMin()));
        this->ui.lbl_y_max_value->setText(text_number.asprintf("%.3f", coord_manager->getYMax()));
        this->ui.lbl_z_max_value->setText(text_number.asprintf("%.3f", coord_manager->getZMax()));

        // Print over the offset label if the drawing has been moved
        if (coord_manager->getOffsetX() > 0 && coord_manager->getOffsetY() > 0)
        {
            this->ui.lbl_offset_value->setText("X - Y");
        }
        else if (coord_manager->getOffsetX() > 0 && coord_manager->getOffsetY() == 0)
        {
            this->ui.lbl_offset_value->setText("X");
        }
        else if (coord_manager->getOffsetX() == 0 && coord_manager->getOffsetY() > 0)
        {
            this->ui.lbl_offset_value->setText("Y");
        }
        else
        {
            this->ui.lbl_offset_value->setText(tr("No"));
        }

        // Drawing size
        float drawing_w = coord_manager->getXMax() - coord_manager->getXMin();
        float drawing_h = coord_manager->getYMax() - coord_manager->getYMin();

        // Print the drawing size on its label
        this->ui.lbl_rectangle_value->setText(
            text_number.asprintf("%.3f", drawing_w) + " X " +
            text_number.asprintf("%.3f", drawing_h)
        );

        // Progress dialog to show while processing the list of coordinates
        QProgressDialog progress_dialog = QProgressDialog(tr("Elaborazione primitive"),
            tr("Annulla"), 0, num_coords - 1, this);
        progress_dialog.setModal(true);
        progress_dialog.setMinimumDuration(0);

        /* 
            It is not convenient to update the progress bar at each loop iteration, that would
            result in a very slow execution, this sets the update to be executed once every
            1 / 200 of the total iterations.
        */
        int progress_step = int(num_coords / 200);

        for (int i = 0; i < num_coords; i++)
        {
            // If 1 / 200 of the instructions have been processed
            if (progress_step != 0 && i % progress_step == 0)
            {
                // Update the progress dialog
                progress_dialog.setValue(i);

                // If the cancel button of the progress dialog was clicked
                if (progress_dialog.wasCanceled())
                {
                    // Abort the drawing resetting the changed variables
                    this->cancelDrawing();
                    // Stop the loop
                    break;
                }
            }

            /* 
                If this is not the end of the file and the tool must be raised
                that means that the next movement will be to position the tool.
            */
            if (i < num_coords && coords[i][0] == coord_manager->getUp())
            {
                // Add the vertical movement lenght to the positioning distance
                positioning_dst += abs(this->last_z);

                // Set the flags
                lowering = false;
                drawing = false;
                repositioning = true;

                // Terminate the iteration
                continue;
            }
            /* 
                If this is the end of the file and the tool must be raised,
                the only distance to considerate is the vertical movement, then everything is over.
            */
            else if (coords[i][0] == coord_manager->getUp())
            {
                // Add the the absolute last z value to the positioning distance
                positioning_dst += abs(this->last_z);

                // Update the current position
                current_position = QVector3D(current_position[0], current_position[1], 0);

                // Set the flags
                lowering = false;
                drawing = false;
                repositioning = false;

                // Terminate the iteration
                continue;
            }

            /* 
                If the tool must be lowered(no need to check if we are at the end of the file
                because if the tool lowerd, for sure there is something else to do).
            */
            if (coords[i][0] == coord_manager->getDown())
            {
                /* 
                    I do not update the Z distance now, because I should read the next tuple to know it.
                    I postpone that to the next iteration, which reading lowering = True
                    will know that it has to calculate the distance.
                */

                // Set the flags
                lowering = true;
                drawing = false;
                repositioning = false;

                // Terminate the iteration
                continue;
            }

            // From here on, there will only be vectors with coordinates and not movement indication

            // If the tool is repositioning on the XY plane
            if (repositioning)
            {
                dx = pow(coords[i][0] - current_position[0], 2);
                dy = pow(coords[i][1] - current_position[1], 2);
                positioning_dst += sqrt(dx + dy);

                // Update the current position, Z position does not change over repositionings
                current_position = QVector3D(coords[i][0], coords[i][1], current_position[2]);

                // Set the flags
                lowering = false;
                drawing = false;
                repositioning = false;

                // Terminate the iteration
                continue;
            }

            // If the tool is lowering
            if (lowering)
            {
                // Here there will certainly be a tuple containing only a Z coordinate changing
                positioning_dst += abs(coords[i][2]);

                // Update the current position
                current_position = QVector3D(current_position[0], current_position[1], coords[i][2]);

                // Set the flags
                lowering = false;
                drawing = true;
                repositioning = false;

                // Terminate the iteration
                continue;
            }

            // If the tool is engraving
            if (drawing)
            {
                // Calculate the engraving distance
                dx = pow(coords[i][0] - current_position[0], 2);
                dy = pow(coords[i][1] - current_position[1], 2);
                dz = pow(coords[i][2] - current_position[2], 2);
                engraving_dst += sqrt(dx + dy + dz);

                /* 
                    Draw the segment until the point indicated by the coordinate
                    X stays as read from the file, the Y must be calculated because the Y tha machine's axis
                    values increas from bottom to top, in the canvas instead goes from top to bottom.
                */
                p1 = QPoint(
                    current_position[0] * this->scale_factor,
                    this->scene_h - (current_position[1] * this->scale_factor)
                );

                p2 = QPoint(
                    coords[i][0] * this->scale_factor,
                    this->scene_h - (coords[i][1] * this->scale_factor)
                );

                if (draw_gradient)
                {
                    start_mapped_color = this->mapRange(
                        abs(current_position[2]), 10, abs_z_max, 0, 255);

                    end_mapped_color = this->mapRange(
                        abs(coords[i][2]), 10, abs_z_max, 0, 255);

                    // If the user requires to draw the segments using colors
                    if (draw_color)
                    {
                        // Yellow to Blue color
                        start_color.setRgb(
                            255 - start_mapped_color, 255 - start_mapped_color, start_mapped_color);

                        end_color.setRgb(
                            255 - end_mapped_color, 255 - end_mapped_color, end_mapped_color);
                    }
                    else
                    {
                        // Grayscale
                        start_color.setRgb(
                            255 - start_mapped_color, 255 - start_mapped_color, 255 - start_mapped_color);

                        end_color.setRgb(
                            255 - end_mapped_color, 255 - end_mapped_color, 255 - end_mapped_color);
                    }

                    gradient = QLinearGradient(p1, p2);
                    gradient.setColorAt(0, start_color);
                    gradient.setColorAt(1, end_color);

                    pen = QPen(QBrush(gradient), 1);
                }
                else
                {
                    if (draw_color)
                    {
                        color = QColor(0, 0, 255);
                        pen = QPen(QBrush(color), 1);
                    }
                    else
                    {
                        color = QColor(0, 0, 0);
                        pen = QPen(QBrush(color), 1);
                    }
                }

                // Add the segment to the scene
                this->scene->addLine(QLine(p1, p2), pen = pen);

                // Update the current position
                current_position = QVector3D(coords[i][0], coords[i][1], coords[i][2]);

                // Update the last z value
                this->last_z = coords[i][2];

                // Update the lines count
                lines += 1;

                // Set the flags
                lowering = false;
                drawing = true;
                repositioning = false;

                // Terminate the iteration
                continue;
            }
        }

        // If the Z Max checkbox is checked
        if (this->ui.chk_zmax->isChecked())
        {
            QColor color_hl = QColor(255, 0, 0);
            QPen pen_hl = QPen(QBrush(color_hl), 3);

            // Loop through the z_max_depth list
            foreach (QVector2D point, coord_manager->getZMaxList())
            {
                // Draw the circle
                this->scene->addEllipse(
                    QRect(
                        (point.x() * this->scale_factor) - this->ZMAX_CIRCLE_RADIUS,
                        (this->scene_h - (point.y() * this->scale_factor)) - this->ZMAX_CIRCLE_RADIUS,
                        2 * this->ZMAX_CIRCLE_RADIUS,
                        2 * this->ZMAX_CIRCLE_RADIUS
                    ), 
                    pen_hl
                );
            }
        }

        progress_dialog.setValue(num_coords - 1);
        // Says that there is something on the scene
        this->iso_drawn = true;
        // Estimate the woking time in seconds
        int working_time = this->workingTime(engraving_dst, positioning_dst);

        // Print the estimated working time to the relative label formatted as hh:mm:ss
        this->ui.lbl_working_time_value->setText(Helpers::secondsToTimestring(working_time));

        // Print the engravingand repositioning distances to the relative labels
        this->ui.lbl_eng_dst_value->setText(text_number.asprintf("%.3f", engraving_dst));
        this->ui.lbl_pos_dst_value->setText(text_number.asprintf("%.3f", positioning_dst));

        /* 
            If the flag is set to True, that means that the drawing has ben regenerated
            after that the scene was resized
        */
        if (this->resize_timer_running)
        {
            // Says that the timer timed outand it not running anymore
            this->resize_timer_running = false;
        }
    }
}

/********** PROTECTED FUNCTIONS **********/

void IsoViewer::changeEvent(QEvent* event)
{
    if (0 != event)
    {
        switch (event->type())
        {
        // This event is sent if a translator is loaded
        case QEvent::LanguageChange:
            ui.retranslateUi(this);
            break;

        // This event is sent if the system language changes
        case QEvent::LocaleChange:
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            this->loadLanguage(locale);
        }
        break;
        }
    }

    QMainWindow::changeEvent(event);
}

/********** PROTECTED SLOTS **********/

void IsoViewer::slotLanguageChanged(QAction* action)
{
    if (0 != action)
    {
        // Load the language dependent on the action content
        loadLanguage(action->data().toString());

        // Update the config file
        this->updateOptions("lang", action->data().toString().toStdString());
    }
}


/********** PRIVATE FUNCTIONS **********/

void IsoViewer::loadLanguage(const QString& rLanguage)
{
    if (this->m_currLang != rLanguage)
    {
        this->m_currLang = rLanguage;
        QLocale locale = QLocale(this->m_currLang);
        QLocale::setDefault(locale);
        QString languageName = QLocale::languageToString(locale.language());
        switchTranslator(this->m_translator, QString("isoviewer_%1.qm").arg(rLanguage));
        switchTranslator(this->m_translatorQt, QString("qt_%1.qm").arg(rLanguage));
    }
}

void IsoViewer::switchTranslator(QTranslator& translator, const QString& filename)
{
    // Remove the old translator
    qApp->removeTranslator(&translator);

    // Load the new translator
    QString path = QApplication::applicationDirPath();
    path.append("/languages/");

    // Here Path and Filename has to be entered because the system didn't find the QM Files
    if (translator.load(path + filename))
        qApp->installTranslator(&translator);
}

void IsoViewer::createLanguageMenu()
{
    QActionGroup* langGroup = new QActionGroup(this->ui.menuLingua);
    langGroup->setExclusive(true);

    connect(langGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotLanguageChanged(QAction*)));

    /*
        Build the path to the folder containing the language files.
        It is the folder where this software is stored, 
        and from there inside the "language" subfolder.
    */
    this->m_langPath = QApplication::applicationDirPath();
    this->m_langPath.append("/languages");

    QDir dir(this->m_langPath);
    // List of the language files
    QStringList language_files = dir.entryList(QStringList("isoviewer_*.qm"));

    for (int i = 0; i < language_files.size(); ++i)
    {
        // Get locale extracted by filename
        QString locale;
        locale = language_files[i]; // "e.g. isoviewer_it.qm"
        locale.truncate(locale.lastIndexOf('.')); // "e.g. isoviewer_it"
        locale.remove(0, locale.lastIndexOf('_') + 1); // "e.g. it"

        QString lang = QLocale::languageToString(QLocale(locale).language());
        /*
            Load the icon file, it is inside the same language folder
            and the name is <languagecode>.png
        */
        QIcon ico(QString("%1/%2.png").arg(this->m_langPath).arg(locale));

        QAction* action = new QAction(ico, lang, this);
        action->setCheckable(true);
        action->setData(locale);

        this->ui.menuLingua->addAction(action);
        langGroup->addAction(action);

        // Highlight, inside the menu, the current language
        if (this->language == locale)
        {
            action->setChecked(true);
        }
    }
}
