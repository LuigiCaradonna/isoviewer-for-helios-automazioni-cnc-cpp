#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>
#include <QProgressDialog>
#include <fstream>
#include <iostream>
#include <QVector2D>
#include <QVector3D>
#include <QFile>
#include <QTimer>
#include "json.hpp"
#include "ui_IsoViewer.h"

using json = nlohmann::json;

class MyGraphicScene : public QGraphicsScene
{
    Q_OBJECT

protected:
    /*
     * Override of the method mouseMoveEvent to emit a custom signal.
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
    {
        // Mouse pointer position over the scene
        QPointF pos = event->scenePos();

        // Emit the signal providing the mouse position as parameter
        emit signalMousePos(pos);
    }

signals:
    /*
     * Signal emitted when the mouse moves over the scene.
     */
    void signalMousePos(QPointF pos);
};

class IsoViewer : public QMainWindow
{
    Q_OBJECT

public:
    /********** CONSTRUCTOR **********/

    /*
     * Construct the IsoViewer object.
     *
     * @param	QWidget*	parent	- Parent widget
     */
    IsoViewer(QWidget *parent = Q_NULLPTR);


    /********** DESTRUCTOR **********/

    /*
     * Destructs the IsoViewer object.
     */
    ~IsoViewer();


    /********** OVERRIDES **********/

    /*
     * Overrides the QMainWindow resizeEvent() called when the window is resized.
     *
     * @param   QResizeEvent*    event
     * 
     * @return void
     */
    void resizeEvent(QResizeEvent* event) override;

    /*
     * Overrides the QMainWindow showEvent() called when the window is shown.
     *
     * @param   QShowEvent*    event
     * 
     * @return void
     */
    void showEvent(QShowEvent* event) override;


    /********** PUBLIC FUNCTIONS **********/


    /********** INIT FUNCTIONS **********/

    /*
     * Reads the config file and sets the corresponding checkboxes.
     * 
     * @return void
     */
    void initOptions();


    /********** UPDATE FUNCTIONS **********/

    /*
     * Updates the value of config file relative to the provided key.
     * 
     * @param   const std::string&      key    - Name of the option to change.
     * @param   std::string             value  - Value to set.
     * @param   std::string             type   - Type of the value "int", "string"...
     * 
     * @return void
     */
    void updateOptions(const std::string& key, std::string value, std::string type);

    /*
     * Toggles the setting value for the fit option inside the config file.
     * 
     * @return void
     */
    void toggleFit();

    /*
     * Toggles the setting value for the autoresize option inside the config file.
     * 
     * @return void
     */
    void toggleAutoresize();

    /*
     * Toggles the setting value for the color option inside the config file.
     * 
     * @return void
     */
    void toggleColor();

    /*
     * Toggles the setting value for the gradient option inside the config file.
     * 
     * @return void
     */
    void toggleGradient();

    /*
     * Updates the last folder used to load a file.
     *
     * @param   const QString&      f      - Path to the folder
     * 
     * @return void
     */
    void updateFolder(const QString & f);


    /********** ACCESSORS **********/

    /*
     * Gets and shows the mouse pointer coordinates.
     *
     * @param   const QPointF&     pos - Mouse position as point
     * 
     * @return void
     */
    void mousePosition(const QPointF& pos);

    /*
     * Returns the visible size of the canvas, applying a compensation 
     * for the px difference introduced by the antialiasing if necessary.
     *
     * @return QVector2D - The size of the canvas
     */
    QVector2D getCanvasSize();


    /********** MODIFIERS **********/


    /********** RESETTERS **********/

    /*
     * Resets all the input fields, the errors  and the scene.
     *
     * @return void
     */
    void fullReset();

    /*
     * Resets the coordinates' min and max values and the offsets.
     *
     * @return void
     */
    void resetCoordinatesLimits();

    /*
     * Resets the input fields, the selected files label and the status bar to
     * remove eventual error messages and relative highlighting.
     *
     * @return void
     */
    void resetErrors();

    /*
     * Reset the scene to the initial state.
     * 
     * @param   bool    reset_to_draw - If false also reset the selected files.
     *
     * @return void
     */
    void resetScene(bool reset_to_draw = false);

    /*
     * Resets the both the scene and the coordinates' limits.
     *
     * @return void
     */
    void cancelDrawing();


    /********** OTHERS **********/

    /*
     * Opens the file browser.
     *
     * @return void
     */
    void browseFile();

    /*
     * Elides a long text to fit the label width.
     *
     * @param   const QLabel&   label    - Label where to write the text.
     * @param   const QString&  text     - Text to write into the label.
     *
     * @return QString  - The elided text
     */
    QString elideText(const QLabel& label, const QString& text);

    /*
     * Validates the data.
     *
     * @return True if the inserted data are correct, false otherwise.
     */
    bool checkData();

    /*
     * Calculates the scale factor to resize the drawing to fit the scene size.
     *
     * @param   int    w    - Width of the slab in use for the job.
     * @param   int    h    - Height of the slab in use for the job.
     *
     * @return True if the data are correct, false otherwise.
     */
    float scaleFactor(int w, int h);

    /*
     * Set the scen to contain the drawing just elaborated.
     *
     * @return True if the data are correct, false otherwise.
     */
    void setScene();

    /*
     * Read the ISO files and gets the coordinates useful for the drawing.
     *
     * @return QList<QVector3D> List of coordinates which will be used for the drawing.
     */
    QList<QVector3D> getCoordinates();

    /*
     * Translate the provided coordinates along x and y axes by the given amount dx and dy.
     * 
     * WARNING: This directly modifies the original vector!
     * 
     * @param   QList<QVector3D>&       coords  - List of coordinates to be traslated.
     * @param   int                     dx      - Offset along x axis.
     * @param   int                     dy      - Offset along y axis.
     * 
     * @return void
     */
    void translateCoords(QList<QVector3D>& coords, int dx, int dy);

    /*
     * Estimates the working time in seconds.
     * 
     * @param const float&     eng_dst     - Engraving distance
     * @param const float&     pos_dst     - Repositioning distance
     *
     * @return int  The estimated time in seconds.
     */
    int workingTime(const float& eng_dst, const float& pos_dst);

    /*
     * Maps a value from a range to another.
     * Here it is used to convert a depth value to a color, the deeper is the engraving, the higher is the color value.
     *
     * @param const float&     value          - Value to be converted.
     * @param const float&     source_min     - Source range min value.
     * @param const float&     source_max     - Source range max value.
     * @param const float&     target_min     - Target range min value.
     * @param const float&     target_max     - Target range max value.
     *
     * @return int  The mapped value.
     */
    int mapRange(const float& value, const float& source_min, const float& source_max, const float& target_min, const float& target_max);

    /*
     * Displays the drawing.
     * 
     * @return void
     */
    void draw();

private:
    // The vector containing the coordinates consists of float values.
    // The following two costants are used to indicate the UP and DOWN positions of the tool.
    // Those two values will never appear into the ISO file generated by the software of the machine.

    // Code for the UP position of the tool
    const float UP = 5000.0f;
    // Code for the DOWN position of the tool
    const float DOWN = 6000.0f;

    // List of strings containing the selected ISO files
    QStringList iso_files;
    // Scale factor to use to resize the drawing
    float scale_factor = 1;

    // Scene width
    int scene_w = 0;
    // Scene height
    int scene_h = 0;

    // Name of the configuration file
    const std::string config_file = "config.cfg";

    // Folder to open when clicking the "Load Files" button, initialized with the value contained into
    // the config file and updated each time a file is loaded from a different folder to always open
    // the last used folder.
    std::string folder;

    // Setting the antialiasing for the canvas, active by default,
    // the canvas area is extended by 4 pixels over the visible area, and the mouse position
    // is offset by 4px, this will compensate this difference.
    // Must be set to 0 if the antialiasing is not in use.
    int canvas_expanded = 4;

    // Min x position where the drawing will be performed
    float x_min = 10000;
    // Min y position where the drawing will be performed
    float y_min = 10000;
    // Max x position where the drawing will be performed
    float x_max = -10000;
    // Max y position where the drawing will be performed
    float y_max = -10000;
    // Max z position where the drawing will be performed (grows negative)
    float z_max = 0;

    // Last Z position, to be updated for each segment, this will be used to know
    // how much the tool must be raised to leave the working surface
    float last_z = 0;

    /*
     * Jobs for the machine could also be placed in negative area, but the canvas only accepts
     * positive coordinates, thus the drawing must be translated to be visible, the following
     * two variables will contain the distance to move the drawing along the x and y axes.
     */

    // Offset along the x axis to get the drawing in the positive area
    int offset_x = 0;
    // Offset along the y axis to get the drawing in the positive area
    int offset_y = 0;

    // Says whether anything was drawn or not
    bool iso_drawn = false;
    
    // Says whether the timer to delay the drawing regeneration is active or not
    bool resize_timer_running = false;
    // Says whether the timer must be used or not
    bool delay_enabled = true;
    // Delay in milliseconds;
    int delay_timeout = 500;

    // Contains the full string of the selected files, only their names
    // If needed there will be another variable containing a cut string returned by elideText()
    QString selected_files_full_string = "";

    // Timer to redraw upon window resize
    QTimer* reset_timer;

    // Scene where the drawing will be displayed
    MyGraphicScene* scene;

    // User interface
    Ui::IsoViewerClass ui;


    /********** PRIVATE FUNCTIONS **********/

    /*
     * Checks if the given string corresponds to an existing file.
     * 
     * @param   const std::string&      name    - Name of the file to check.
     * 
     * @return  bool    true if the file exists, false otherwise.
     */
    bool fileExists(const std::string& name);

    /*
     * Initializes a new config file.
     *
     * @return  void
     */
    void initConfigFile();

    /*
     * Truncates a float to the given number of decimals.
     *
     * @param   const float&    number      - number to truncate
     * @param   int             decimals    - number of wanted decimal digits
     *
     * @return  float    The truncated number.
     */
    float truncToDecimal(const float& number, int decimals);

    /*
     * Formats the number of seconds passed as int to a string h:m:s.
     *
     * @param   const int&      seconds    - Number of seconds to be formatted.
     *
     * @return  QString    The formatted time string.
     */
    QString secondsToTimestring(const int seconds);
};
