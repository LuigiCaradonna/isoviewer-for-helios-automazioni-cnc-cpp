#pragma once

#include "qstring.h"
#include "qlabel.h"
#include <cmath>
#include <filesystem>

class Helpers
{
public:
    /*
     * Truncates a float to the given number of decimals.
     *
     * @param   const float&    number      - number to truncate
     * @param   int             decimals    - number of wanted decimal digits
     *
     * @return  float    The truncated number.
     */
	static float truncToDecimal(const float& number, int decimals);

    /*
     * Formats the number of seconds passed as int to a string h:m:s.
     *
     * @param   const int&      seconds    - Number of seconds to be formatted.
     *
     * @return  QString    The formatted time string.
     */
	static QString secondsToTimestring(const int seconds);

    /*
     * Checks if the given string corresponds to an existing file.
     *
     * @param   const std::string&      s    - String containing the path or path\file to check.
     *
     * @return  bool    true if the file exists, false otherwise.
     */
	static bool pathFileExists(const std::string& s);

    /*
     * Elides a long text to fit the label width.
     *
     * @param   const QLabel&   label    - Label where to write the text.
     * @param   const QString&  text     - Text to write into the label.
     *
     * @return QString  - The elided text
     */
	static QString elideText(const QLabel& label, const QString& text);
};

