#include "CoordManager.h"

/********** CONSTRUCTOR **********/

CoordManager::CoordManager(QObject* parent)
{

}

/********** DESTRUCTOR **********/

CoordManager::~CoordManager()
{

}

/********** GETTERS **********/

float CoordManager::getXMin()
{
    return this->x_min;
}

float CoordManager::getYMin()
{
    return this->y_min;
}

float CoordManager::getXMax()
{
    return this->x_max;
}

float CoordManager::getYMax()
{
    return this->y_max;
}

float CoordManager::getZMax()
{
    return this->z_max;
}

float CoordManager::getUp()
{
    return this->UP;
}

float CoordManager::getDown()
{
    return this->DOWN;
}

float CoordManager::getOffsetX()
{
    return this->offset_x;
}

float CoordManager::getOffsetY()
{
    return this->offset_y;
}

QList<QVector2D> CoordManager::getZMaxList()
{
    return this->z_max_list;
}

/********** PUBLIC FUNCTIONS **********/

void CoordManager::translateCoords(QList<QVector3D>& coords, int dx, int dy)
{
    for (int i = 0; i < coords.length(); ++i)
    {
        if (coords[i][0] != this->UP && coords[i][0] != this->DOWN)
        {
            coords[i][0] = coords[i][0] + dx;
            coords[i][1] = coords[i][1] + dy;
            // coords[i][2], which is the z coordinate, stays the same
        }
    }
}

void CoordManager::translateCoords(QList<QVector2D>& coords, int dx, int dy)
{
    for (int i = 0; i < coords.length(); ++i)
    {
        if (coords[i][0] != this->UP && coords[i][0] != this->DOWN)
        {
            coords[i][0] = coords[i][0] + dx;
            coords[i][1] = coords[i][1] + dy;
        }
    }
}

void CoordManager::resetCoordinatesLimits()
{
    this->x_min = 10000;
    this->y_min = 10000;
    this->z_max = 0;
    this->x_max = 0;
    this->y_max = 0;
    this->offset_x = 0;
    this->offset_y = 0;
}

QList<QVector3D> CoordManager::processCoordinates(const QStringList& iso_files, bool sculpture, bool fit, bool size_given)
{
    // List containing the useful coordinates
    QList<QVector3D> coords;

    // List of all the instructions contained inside the selected files
    QStringList iso;

    // Says if a point has been found into the z_max_list
    bool point_found = false;

    // The position where the engraving starts is after the second G12 Z0
    // the variable is a counter to recognize it
    int z_down = 0;

    // At each iteration, this will contain the current line of code splitted
    QStringList subline;

    // These will contain the x, y and z coordinate to add to the coordinates list
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    foreach(QString f, iso_files)
    {
        QFile file(f);
        // Verify that the file exists and has the proper extension
        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream in(&file);
        while (!in.atEnd())
        {
            // Add the file content to the list of instructions
            iso.append(in.readLine());
        }

        file.close();
    }

    // The PGR files containing a sculpture are slightly different from the general ones.
    // If the file to show contains a sculpture (the user has to tell us)
    if (sculpture)
    {
        int j = 0;
        QList<int> start_position;

        foreach(QString loc, iso)
        {
            // Find the line where an ISO file begins.
            // This is inside a loop because several files could have been queued into one and
            // the 2 lines below must be added after the beginning of each of them.
            if (loc.indexOf("QUOTE RELATIVE") == 0)
            {
                // Add at the beginning of the list
                start_position.insert(0, j);
            }

            j += 1;
        }

        foreach(unsigned int pos, start_position)
        {
            // Add this two lines to adapt it to a general PGR file
            iso.insert(static_cast<qsizetype>(pos) + 5, "G12 Z0");
            iso.insert(static_cast<qsizetype>(pos) + 7, "G02 Z-10");
        }
    }

    int num_rows = iso.length();

    // Progress dialog shown while processing the list of instructions
    QProgressDialog progress_dialog = QProgressDialog(tr("Estrapolazione coordinate"), tr("Annulla"), 0, num_rows - 1);
    progress_dialog.setModal(true);
    progress_dialog.setMinimumDuration(0);

    // It is not convenient to update the progress bar at each loop iteration, that would
    // result in a very slow execution, this sets the update to be executed once every
    // 1 / 500 of the total iterations
    int progress_step = int(num_rows / 500);

    // Counter for the prograss dialog
    int i = 0;

    foreach(QString line_of_code, iso)
    {
        // Reset the subline
        subline.clear();

        // If 1 / 500 of the instructions have been processed
        if (progress_step != 0 && i % progress_step == 0)
        {
            // Update the progress dialog
            progress_dialog.setValue(i);

            // If the cancel button of the progress dialog was clicked
            if (progress_dialog.wasCanceled())
            {
                // Reset the modified variables
                this->resetCoordinatesLimits();

                // Stop the loop and return an empty list
                return QList<QVector3D>();
            }
        }

        // Rows starting with G02 X indicate positions which produce engravings - i.e.: G02 X508 Y556 Z-13
        if (line_of_code.indexOf("G02 X") == 0)
        {
            // Split the code
            subline = line_of_code.split(" ");

            // The second element is the X coordinate, remove the first character (X)
            // limit the number to 3 decimals
            x = Helpers::truncToDecimal(subline[1].mid(1).toFloat(), 3);

            // Update the x min
            if (x < this->x_min)
            {
                this->x_min = x;
            }
            // Update the x max
            if (x > this->x_max)
            {
                this->x_max = x;
            }

            // The third element is the Y coordinate, remove the first character (Y)
            // limit the number to 3 decimals
            y = Helpers::truncToDecimal(subline[2].mid(1).toFloat(), 3);

            // Update the y min
            if (y < this->y_min)
            {
                this->y_min = y;
            }
            // Update the y max
            if (y > this->y_max)
            {
                this->y_max = y;
            }

            // The fourth element is the Z coordinate, remove the first character (Z)
            // limit the number to 3 decimals
            z = Helpers::truncToDecimal(subline[3].mid(1).toFloat(), 3);

            // Update the z max
            if (z < this->z_max)
            {
                this->z_max = z;

                // Clear the z_max_list, because a new max has been found
                this->z_max_list.clear();

                // Insert the new point to the list
                this->z_max_list.append(QVector2D(x, y));
            }
            // This point is at the same depth as the points already in the list
            else if (z == this->z_max)
            {
                // The tool could pass multiple times over the same point, we do not want to highlight
                // multiple times the same point

                // A new loop is starting, reset the variable to false
                point_found = false;

                foreach(QVector2D point, this->z_max_list)
                {
                    // If a point with the same coordinates is already contained by the list
                    if (x == point.x() && y == point.y())
                    {
                        // Update the boolean variable
                        point_found = true;
                        // Stop the loop, no need to iterate further
                        break;
                    }
                }

                // If this point is not already into the list
                if (!point_found)
                {
                    // Add the point to the list
                    this->z_max_list.append(QVector2D(x, y));
                }
            }

            // Add the coordinates to the list
            coords.append(QVector3D(x, y, z));
        }
        // Rows starting with G12 X indicate a repositioning over the XY plane
        else if (line_of_code.indexOf("G12 X") == 0)
        {
            // Considering the positioning, X an Y min / max must not be calculated from where the tool starts
            // or X and Y min would always be 0 since it is the origin of the job, they must be considered
            // from the next position, that is where the the tool lowers for the first time to engrave.
            // It is not sure that this will be the min or max, but it is a valid position to consider.

            // Split the code
            subline = line_of_code.split(" ");

            // The second element is the X coordinate, remove the first character (X)
            // limit the number to 3 decimals
            x = Helpers::truncToDecimal(subline[1].mid(1).toFloat(), 3);

            // The third element is the Y coordinate, remove the first character (Y)
            // limit the number to 3 decimals
            y = Helpers::truncToDecimal(subline[2].mid(1).toFloat(), 3);

            // If the X and Y values are not 0, an engraving will start from this point
            if (x != 0 && y != 0)
            {
                // Update the x min
                if (x < this->x_min)
                {
                    this->x_min = x;
                }

                // Update the x max
                if (x > this->x_max)
                {
                    this->x_max = x;
                }

                // Update the y min
                if (y < this->y_min)
                {
                    this->y_min = y;
                }

                // Update the y max
                if (y > this->y_max)
                {
                    this->y_max = y;
                }
            }

            //Add the coordinates to the list, this will always be preceded by an "up"
            coords.append(QVector3D(x, y, z));
        }
        // Rows starting with G02 Z indicate the only vertical movement to start to engrave
        else if (line_of_code.indexOf("G02 Z") == 0)
        {
            // Split the code
            subline = line_of_code.split(" ");

            // Says that the next coordinate will be the tool lowering
            coords.append(QVector3D(this->DOWN, 0, 0));

            // Says how much the tool lowers
            coords.append(QVector3D(0, 0, Helpers::truncToDecimal(subline[1].mid(1).toFloat(), 3)));
        }
        // Rows starting with G12 Z0 indicate the only vertical movement to raise the tool from the working plane
        else if (line_of_code.indexOf("G12 Z0") == 0)
        {
            /*
             * Example
             * G02 X100 Y0 Z-10
             * G02 X0 Y0 Z-10
             * G12 Z0
             * G12 X150 Y100
             * G02 Z-10
             * G02 X150 Y200 Z-10
             * In this case, the coordinates list would contain
             * (100, 0), (0, 0), (150, 100)
             * but the segment from(0, 0) to(150, 100) must not be drawn because Z=0, thus "UP" will indicate this situation.
             * When an "UP" is found, the last Z coordinate must be read (-10 in this case) to know how much the tool will be raised.
             * The absolute value of the Z must be considered, since the Z is always negative or 0
             */
            coords.append(QVector3D(this->UP, 0, 0));
        }

        i++;
    }

    // Check whether the X and/or Y somewhere become negative and take everything back to positive values
    // or it will not be possible to draw on the scene, which only accepts positive coordinates
    if (this->x_min < 0)
    {
        this->offset_x = abs(this->x_min);
        this->x_min = 0;
        this->x_max += this->offset_x;
    }

    if (this->y_min < 0)
    {
        this->offset_y = abs(this->y_min);
        this->y_min = 0;
        this->y_max += this->offset_y;
    }

    // If at least one of the offset has been set
    if (this->offset_x > 0 || this->offset_y > 0)
    {
        this->translateCoords(coords, this->offset_x, this->offset_y);
        this->translateCoords(this->z_max_list, this->offset_x, this->offset_y);

        // Update the x/y min and max
        // min will be 0, max are the old max plus the absolute value of the old mins
        this->x_max += abs(this->x_min);
        this->y_max += abs(this->y_min);
        this->x_min = 0;
        this->y_min = 0;
    }
    // If the user wants to fit the drawing into the canvas AND didn't provide a size for the slab
    // To check if the size was provided is enough to check for one of them, the validation functions
    // takes care thet both or none are set.
    // There is no need to reposition the drawing in case the slab size was provided, it is always considered
    // positioned on (0, 0)
    else if (fit && !size_given)
    {
        // Translate the drawing to remove the eventual space to the left and under the drawing
        if (this->x_min > 0 || this->y_min > 0)
        {
            this->translateCoords(coords, -this->x_min, -this->y_min);
            this->translateCoords(this->z_max_list, -this->x_min, -this->y_min);

            // Update the x/y min and max
            // min will be 0, max are the old max minus the amount of the translation along the relative axis
            this->x_max -= this->x_min;
            this->y_max -= this->y_min;
            this->x_min = 0;
            this->y_min = 0;
        }
    }

    progress_dialog.setValue(num_rows - 1);

    return coords;
}
