#include "Helpers.h"

float Helpers::truncToDecimal(const float& number, int decimals)
{
	float multiplier = pow(10, decimals);

	return (int)(number * multiplier) / multiplier;
}

QString Helpers::secondsToTimestring(const int seconds)
{
    int minutes = int(seconds / 60);
    int hours = int(minutes / 60);
    int secs = seconds % 60;

    QString hh, mm, ss;

    hh = QString::number(hours);
    if (hours < 10)
        hh = "0" + hh;

    minutes = minutes % 60;
    mm = QString::number(minutes);
    if (minutes < 10)
        mm = "0" + mm;

    ss = QString::number(secs);
    if (secs < 10)
        ss = "0" + ss;

    return hh + ":" + mm + ":" + ss;
}

bool Helpers::pathFileExists(const std::string& s)
{
    struct stat buffer;
    return (stat(s.c_str(), &buffer) == 0);
}

QString Helpers::elideText(const QLabel& label, const QString& text)
{
    // Get the metrix for the font used inside the label
    QFontMetrics metrix = QFontMetrics(label.font());

    // Elide the text at a width of 15px less than the label width to leave some padding
    QString elided_text = metrix.elidedText(text, Qt::ElideRight, label.width() - 15);

    return elided_text;
}
