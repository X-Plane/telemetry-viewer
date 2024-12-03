//
// Created by Sidney on 29/11/2023.
//

#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#include "xplane_installations.h"

#if WIN
	#include <Shlobj.h>
#endif

static QString s_installer_file("x-plane_install_12.txt");

#if WIN

typedef	unsigned char		UTF8;													// Definitions for chars in each of the 3 encoding formats.
typedef	unsigned short		UTF16;													// We will use our normal string for UTF8.  We do not make a
typedef unsigned int		UTF32;													// string for UTF32 because it is only used for 1 char at a time.

typedef std::basic_string<UTF16>	string_utf16;											// Just like an STL string, but made of 16-bit UTF 16 chars.
typedef std::basic_string<UTF32>	string_utf32;

inline int	UTF8_encode(UTF32 c, UTF8 buf[4])
{
	if(c <=    0x7F){ buf[0] =   c &     0xFF				;																										 return 1; }
	if(c <=   0x7FF){ buf[0] = ((c &    0x7C0) >> 6 ) | 0xC0;																			 buf[1] = (c & 0x3F) | 0x80; return 2; }
	if(c <=  0xFFFF){ buf[0] = ((c &   0xF000) >> 12) | 0xE0;										 buf[1] = ((c & 0xFC0) >> 6) | 0x80; buf[2] = (c & 0x3F) | 0x80; return 3; }
	if(c <=0x10FFFF){ buf[0] = ((c & 0x1C0000) >> 18) | 0xF0; buf[1] = ((c & 0x3F000) >> 12) | 0x80; buf[2] = ((c & 0xFC0) >> 6) | 0x80; buf[3] = (c & 0x3F) | 0x80; return 4; }

	return c;
}

inline const UTF16 * UTF16_decode(const UTF16 * chars, UTF32& result)
{
	UTF16 w1 = *chars;
	if (w1 < 0xD800 || w1 > 0xDBFF)		// W1 not in surrogate pair1 range? Jsut return it
	{
		result = w1;
		return chars+1;
	}

	++chars;							// Grab w2 and advance
	UTF16 w2 = *chars;
	++chars;
	result = (((w1 & 0x3FF) << 10) | (w2 & 0x3FF)) + 0x10000;	// Low 10 bits of W1 is high 10 bits, Low 10 bits of W2 is low 10 bits.  Whole thing must be advanced past BMP.
	return chars;
}

void string_utf_16_to_8(const string_utf16 &input, std::string &output)
{
	output.clear();

	const UTF16 * p = (const UTF16 *)input.c_str();
	const UTF16 * e = p + input.size();
	while(p < e)
	{
		UTF32 c;
		p = UTF16_decode(p,c);
		UTF8 b[4];

		int n = UTF8_encode(c,b);
		output.insert(output.end(), b, b + n);
	}
}

#endif


QString xplane_installer_get_base_path()
{
#if WIN
    // Pawel says: I think we could try to use QStandardPaths here although this would need verifying since I think Qt
    // tries to provide per-app path under standard locations on Windows; https://doc.qt.io/qt-5/qstandardpaths.html

	WCHAR win_path[MAX_PATH+1];

	if(SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, win_path)))
	{
		std::string result;

		string_utf_16_to_8((UTF16 *)win_path, result);
		result += '/';

		return QString(result.c_str()) + s_installer_file;
	}

	return QString("C://") + s_installer_file;
#elif APL
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/" + s_installer_file;
#else
	return QDir::homePath() + "/.x-plane/" + s_installer_file;
#endif
}

void parse_install_executables(xplane_installation &installation)
{
	QDir directory(installation.path);
	QFileInfoList file_infos = directory.entryInfoList();

	for(auto &file : file_infos)
	{
		if(file.fileName().contains("Installer"))
			continue;

#if WIN
		if(file.isFile() && file.fileName().startsWith("X-Plane") && file.fileName().endsWith("exe"))
		{
			installation.executables.push_back(file.fileName());
		}
#elif APL
        if (file.isDir() && file.fileName().startsWith("X-Plane") && file.fileName().endsWith("app"))
        {
            QDir subdir(file.filePath() + "/" + "Contents/MacOS");
            QFileInfoList subentries = subdir.entryInfoList();
            // e.g. X-Plane_NODEV_OPT.app/Contents/MacOS/X-Plane_NODEV_OPT
            for (auto const &subentry : subentries) {
                if (subentry.isFile() && subentry.fileName().startsWith("X-Plane")) {
                    auto name = file.fileName() + "/Contents/MacOS/" + subentry.fileName();
                    installation.executables.push_back(name);
                    break;
                }
            }
        }
#else
		if(file.isFile() && file.fileName().startsWith("X-Plane") && file.fileName().endsWith("-x86_64"))
		{
			installation.executables.push_back(file.fileName());
		}
#endif
	}
}

QVector<xplane_installation> get_xplane_installations()
{
	QVector<xplane_installation> result;
	QFileInfo info(xplane_installer_get_base_path());

	if(info.exists() && info.isFile())
	{
		QFile file(info.filePath());

		if(file.open(QIODevice::ReadOnly))
		{
			const size_t length = file.bytesAvailable();

			char *data = new char[length];
			file.read(data, length);
			file.close();

			char *start_token = data;
			char *token = start_token;

			while(token < data + length)
			{
				if(!isprint(*token))
				{
					if(token > start_token)
					{
						*token = '\0';

						QString install_path = QString(start_token);
						QFileInfo path_info(install_path);

						if(path_info.exists() && path_info.isDir())
						{
							xplane_installation install;
							install.path = path_info.path();
							install.telemetry_path = install.path + "/Output/diagnostic reports";
							install.replay_path = install.path + "/Output/replays";

							parse_install_executables(install);
							result.push_back(install);
						}
					}

					start_token = token + 1;
				}

				token ++;
			}

			delete[] data;
		}
	}

	return result;
}

QString xplaneify_path(const QString &full_path)
{
#if WIN
	// X-Plane stupidly will not accept / as a separator after the drive name
	if(full_path.length() > 3)
	{
		if(full_path[1] == ':' && full_path[2] == '/')
		{
			QString result = full_path;
			result[2] = '\\';

			return result;
		}
	}
#endif

	return full_path;
}
