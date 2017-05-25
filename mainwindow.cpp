#include <iostream>
#include <set>
#include <map>
#include <algorithm>
#include <cctype>
#include <string>
#include <sstream>
#include <windows.h>
#include <regex>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "genie/dat/DatFile.h"
#include "genie/lang/LangFile.h"
#include "paths.h"
#include "conversions.h"
#include "wololo/datPatch.h"
#include "wololo/Drs.h"
#include "fixes/berbersutfix.h"
#include "fixes/vietfix.h"
#include "fixes/demoshipfix.h"
#include "fixes/portuguesefix.h"
#include "fixes/malayfix.h"
#include "fixes/ethiopiansfreepikeupgradefix.h"
#include "fixes/maliansfreeminingupgradefix.h"
#include "fixes/ai900unitidfix.h"
#include "fixes/hotkeysfix.h"
#include "fixes/disablenonworkingunits.h"
#include "fixes/feitoriafix.h"
#include "fixes/burmesefix.h"
#include "fixes/incafix.h"
#include "fixes/smallfixes.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include <QWhatsThis>
#include <QPoint>

#include "JlCompress.h"

namespace fs = boost::filesystem;

fs::path HDPath;
fs::path outPath;
std::string const version = "2.1";
std::string language;
std::map<std::string, std::string> translation;
bool secondAttempt = false;

fs::path nfzUpOutPath;
fs::path nfzOutPath;
fs::path modHkiOutPath;
fs::path modHki2OutPath;
fs::path upHkiOutPath;
fs::path upHki2OutPath;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	language = "en";
	ui->setupUi(this);
	HDPath = getHDPath();
	outPath = getOutPath(HDPath);
	changeLanguage(language);

	nfzUpOutPath = outPath / "Games/WololoKingdoms/Player.nfz";
	nfzOutPath = outPath / "Voobly Mods/AOC/Data Mods/WololoKingdoms/Player.nfz";
	modHkiOutPath = outPath / "Voobly Mods/AOC/Data Mods/WololoKingdoms/player1.hki";
	modHki2OutPath = outPath / "Voobly Mods/AOC/Data Mods/WololoKingdoms/player2.hki";
	upHkiOutPath = outPath / "Games/WololoKingdoms/player1.hki";
	upHki2OutPath = outPath / "Games/WololoKingdoms/player2.hki";

	QObject::connect( this->ui->languageChoice, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, [this]() {
		switch(this->ui->languageChoice->currentIndex()) {
			case 0: language = "br"; break;
			case 1: language = "de"; break;
			case 2: language = "en"; break;
			case 3: language = "es"; break;
			case 4: language = "fr"; break;
			case 5: language = "it"; break;
			case 6: language = "jp"; break;
			case 7: language = "ko"; break;
			case 8: language = "nl"; break;
			case 9: language = "ru"; break;
			case 10: language = "zh"; break;
			default: language = "en";
		}
		changeLanguage(language);
		if(this->ui->languageChoice->currentIndex() != 2) {
			this->ui->replaceTooltips->setEnabled(false);
			this->ui->replaceTooltips->setChecked(false);
		} else {
			this->ui->replaceTooltips->setEnabled(true);
		}
	} );
	
	//TODO do this in a loop
	this->ui->hotkeyTip->setIcon(QIcon("resources/question.png"));
	this->ui->hotkeyTip->setIconSize(QSize(16,16));
	this->ui->hotkeyTip->setWhatsThis(translation["hotkeyTip"].c_str());
	QObject::connect( this->ui->hotkeyTip, &QPushButton::clicked, this, [this]() {
			QWhatsThis::showText(this->ui->hotkeyTip->mapToGlobal(QPoint(0,0)),this->ui->hotkeyTip->whatsThis());
	} );
	if(fs::exists("player1.hki")) {
		this->ui->hotkeyChoice->setDisabled(true);
		this->ui->hotkeyChoice->setItemText(0,translation["customHotkeys"].c_str());
		this->ui->hotkeyTip->setDisabled(true);
	}

	this->ui->tooltipTip->setIcon(QIcon("resources/question.png"));
	this->ui->tooltipTip->setIconSize(QSize(16,16));
	this->ui->tooltipTip->setWhatsThis(translation["tooltipTip"].c_str());
	QObject::connect( this->ui->tooltipTip, &QPushButton::clicked, this, [this]() {
		QWhatsThis::showText(this->ui->tooltipTip->mapToGlobal(QPoint(0,0)),this->ui->tooltipTip->whatsThis());
	} );

	this->ui->exeTip->setIcon(QIcon("resources/question.png"));
	this->ui->exeTip->setIconSize(QSize(16,16));
	std::string line = translation["exeTip"];
	boost::replace_all(line, "<folder>", outPath.string()+"\\age2_x1");
	this->ui->exeTip->setWhatsThis(line.c_str());
	QObject::connect( this->ui->exeTip, &QPushButton::clicked, this, [this]() {
		QWhatsThis::showText(this->ui->exeTip->mapToGlobal(QPoint(0,0)),this->ui->exeTip->whatsThis());
	} );
	this->ui->modsTip->setIcon(QIcon("resources/question.png"));
	this->ui->modsTip->setIconSize(QSize(16,16));
	this->ui->modsTip->setWhatsThis(translation["modsTip"].c_str());
	QObject::connect( this->ui->modsTip, &QPushButton::clicked, this, [this]() {
			QWhatsThis::showText(this->ui->modsTip->mapToGlobal(QPoint(0,0)),this->ui->modsTip->whatsThis());
	} );
	this->ui->mapsTip->setIcon(QIcon("resources/question.png"));
	this->ui->mapsTip->setIconSize(QSize(16,16));
	this->ui->mapsTip->setWhatsThis(translation["mapsTip"].c_str());
	QObject::connect( this->ui->mapsTip, &QPushButton::clicked, this, [this]() {
			QWhatsThis::showText(this->ui->mapsTip->mapToGlobal(QPoint(0,0)),this->ui->mapsTip->whatsThis());
	} );
	QObject::connect( this->ui->runButton, &QPushButton::clicked, this, &MainWindow::run);

	this->ui->label->setText(("WololoKingdoms ver. " + version).c_str());
	if(!fs::exists(HDPath/"EmptySteamDepot")) { //This checks whether at least either AK or FE is installed, no way to check for all DLCs unfortunately.
		this->ui->runButton->setDisabled(true);
		return;
	}

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeLanguage(std::string language) {

	std::string line;
	std::ifstream translationFile("resources/"+language+".txt");
	while (std::getline(translationFile, line)) {
		boost::replace_all(line, "\\n", "\n");
		int index = line.find('=');
		translation[line.substr(0, index)] = line.substr(index+1, std::string::npos);
	}
	this->ui->runButton->setText(translation["runButton"].c_str());
	this->ui->replaceTooltips->setText(translation["replaceTooltips"].c_str());
	this->ui->createExe->setText(translation["createExe"].c_str());
	this->ui->useGrid->setText(translation["useGrid"].c_str());
	this->ui->usePw->setText(translation["usePw"].c_str());
	this->ui->useWalls->setText(translation["useWalls"].c_str());
	this->ui->hotkeyChoice->setItemText(1,translation["hotkeys1"].c_str());
	this->ui->hotkeyChoice->setItemText(2,translation["hotkeys2"].c_str());
	this->ui->hotkeyChoice->setItemText(3,translation["hotkeys3"].c_str());

	fs::path vooblyDir = outPath / "Voobly Mods/AOC/Data Mods/WololoKingdoms/";
	fs::path nfzOutPath = vooblyDir / "Player.nfz";

	if(fs::exists(nfzOutPath)) {
		this->ui->hotkeyChoice->setItemText(0,translation["hotkeys0"].c_str());
	} else {
		this->ui->hotkeyChoice->setItemText(0,translation["hotkeyChoice"].c_str());
		this->ui->runButton->setDisabled(true);
		QObject::connect( this->ui->hotkeyChoice, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, [this]{
			if (this->ui->hotkeyChoice->currentIndex() != 0)
				this->ui->runButton->setDisabled(false);
			else
				this->ui->runButton->setDisabled(true);
		} );
	}
	qApp->processEvents();
}

void MainWindow::recCopy(fs::path const &src, fs::path const &dst, bool skip, bool force) {
	// recursive copy
	//fs::path currentPath(current->path());
	if (fs::is_directory(src)) {
		if(!fs::exists(dst)) {
			fs::create_directories(dst);
		}
		for (fs::directory_iterator current(src), end;current != end; ++current) {
			fs::path currentPath(current->path());
			recCopy(currentPath, dst / currentPath.filename(), skip, force);
		}
	}
	else {
		if (skip) {
			boost::system::error_code ec;
			fs::copy_file(src, dst, ec);
		} else if (force)
			fs::copy_file(src,dst,fs::copy_option::overwrite_if_exists);
		else
			fs::copy_file(src, dst);
	}
}

void MainWindow::listAssetFiles(fs::path path, std::vector<std::string> *listOfSlpFiles, std::vector<std::string> *listOfWavFiles) {
	const std::set<std::string> exclude = {
		// Exclude Forgotten Empires leftovers
		"50163", // Forgotten Empires loading screen
		"50189", // Forgotten Empires main menu
		"53207", // Forgotten Empires in-game logo
		"53208", // Forgotten Empires objective window
		"53209" // ???
	};

	for (fs::directory_iterator end, it(path); it != end; it++) {
		std::string fileName = it->path().stem().string();
		if (exclude.find(fileName) == exclude.end()) {
			std::string extension = it->path().extension().string();
			if (extension == ".slp") {
				listOfSlpFiles->push_back(fileName);
			}
			else if (extension == ".wav") {
				listOfWavFiles->push_back(fileName);
			}
		}
	}

}

void MainWindow::convertLanguageFile(std::ifstream *in, std::ofstream *iniOut, genie::LangFile *dllOut, bool generateLangDll, std::map<int, std::string> *langReplacement) {
	std::string line;
	while (std::getline(*in, line)) {
		int spaceIdx = line.find(' ');
		std::string number = line.substr(0, spaceIdx);
		int nb;
		try {
			nb = stoi(number);
			if (nb == 0xFFFF) {
				/*
				 * this one seems to be used by AOC for dynamically-generated strings
				 * (like market tributes), maybe it's the maximum the game can read ?
				*/
				continue;
			}
			if (nb >= 20150 && nb <= 20167) {
				// skip the old civ descriptions
				continue;
			}
			/*
			 * Conquerors AI names start at 5800 (5800 = 4660+1140, so offset 1140 in the xml file)
			 * However, there's only space for 10 civ AI names. We'll shift AI names to 11500+ instead (offset 6840 or 1140+5700)
			 */

			if (nb >= 5800 && nb < 6000) {
				nb += 5700;
				number = std::to_string(nb);
			}
			if (nb >= 106000 && nb < 106160) { //AK&AoR AI names have 10xxxx id, get rid of the 10, then shift
				nb -= 100000;
				nb += 5700;
				number = std::to_string(nb);
			}

			if (nb >= 120150 && nb <= 120180) { // descriptions of the civs in the expansion
				//These civ descriptions can be too long for the tech tree, we'll take out some newlines
				if (nb == 120156 || nb == 120155) {
					boost::replace_all(line, "civilization \\n\\n", "civilization \\n");
				}
				if (nb == 120167) {
					boost::replace_all(line, "civilization \\n\\n", "civilization \\n");
					boost::replace_all(line, "\\n\\n<b>Unique Tech", "\\n<b>Unique Tech");
				}
				// replace the old descriptions of the civs in the base game
				nb -= 100000;
				number = std::to_string(nb);
			}
		}
		catch (std::invalid_argument const & e){
			continue;
		}

		auto strReplace = langReplacement->find(nb);
		if (strReplace != langReplacement->end()) {
			// this string has been changed by one of our patches (modified attributes etc.)
			line = strReplace->second;
		}
		else {
			// load the string from the HD edition file
			int firstQuoteIdx = spaceIdx;
			do {
				firstQuoteIdx++;
			} while (line[firstQuoteIdx] != '"');
			int secondQuoteIdx = firstQuoteIdx;
			do {
				secondQuoteIdx++;
			} while (line[secondQuoteIdx] != '"');
			line = line.substr(firstQuoteIdx + 1, secondQuoteIdx - firstQuoteIdx - 1);
		}

		//convert UTF-8 into ANSI

		std::wstring wideLine = strtowstr(line);
		std::string outputLine;
		//if(language!="zh")
			ConvertUnicode2CP(wideLine.c_str(), outputLine, CP_ACP);
		//else
		//	ConvertUnicode2CP(wideLine.c_str(), outputLine, 1386);


		*iniOut << number << '=' << outputLine <<  std::endl;

		if (generateLangDll) {
			boost::replace_all(outputLine, "·", "\xb7"); // Dll can't handle that character.
			boost::replace_all(outputLine, "\\n", "\n"); // the dll file requires actual line feed, not escape sequences
			try {
				dllOut->setString(nb, outputLine);
			}
			catch (std::string const & e) {
				boost::replace_all(outputLine, "\xb7", "-"); // non-english dll files don't seem to like that character
				dllOut->setString(nb, outputLine);
			}
		}

	}
}

void MainWindow::makeDrs(std::string const inputDir, std::string const moddedInputDir, std::ofstream *out) {

	this->ui->label->setText((translation["working"]+"\n"+translation["workingDrs"]).c_str());
	this->ui->label->repaint();
	const int numberOfTables = 2; // slp and wav

	std::vector<std::string> slpFilesNames;
	std::vector<std::string> wavFilesNames;
	std::vector<std::string> moddedFilesNames;
	listAssetFiles(inputDir, &slpFilesNames, &wavFilesNames);
	listAssetFiles(moddedInputDir, &moddedFilesNames, NULL);

	int numberOfSlpFiles = 0;
	std::vector<std::string>::iterator modIt = moddedFilesNames.begin();
	for (std::vector<std::string>::iterator it = slpFilesNames.begin(); it != slpFilesNames.end();) {
		int comp = (modIt != moddedFilesNames.end())?(*modIt).compare(*it):1;
		if(comp == 0) {
			modIt++;
			it++;
		} else if (comp < 0) {
			modIt++;
		} else
			it++;
		numberOfSlpFiles++;
	}
	int numberOfWavFiles = wavFilesNames.size();
	int offsetOfFirstFile = sizeof (wololo::DrsHeader) +
			sizeof (wololo::DrsTableInfo) * numberOfTables +
			sizeof (wololo::DrsFileInfo) * (numberOfSlpFiles + numberOfWavFiles);
	int offset = offsetOfFirstFile;


	// file infos

	std::vector<wololo::DrsFileInfo> slpFiles;
	std::vector<wololo::DrsFileInfo> wavFiles;

	modIt = moddedFilesNames.begin();
	for (std::vector<std::string>::iterator it = slpFilesNames.begin(); it != slpFilesNames.end();) {
		wololo::DrsFileInfo slp;
		int comp = (modIt != moddedFilesNames.end())?(*modIt).compare(*it):1;
		size_t size;
		if(comp == 0) {
			size = fs::file_size(moddedInputDir + *modIt + ".slp");
			slp.file_id = stoi(*modIt);
			modIt++;
			it++;
		} else if (comp < 0) {
			size = fs::file_size(moddedInputDir + *modIt + ".slp");
			slp.file_id = stoi(*modIt);
			modIt++;
		} else {
			size = fs::file_size(inputDir + *it + ".slp");
			slp.file_id = stoi(*it);
			it++;
		}
		slp.file_data_offset = offset;
		slp.file_size = size;
		offset += size;
		slpFiles.push_back(slp);
	}
	for (std::vector<std::string>::iterator it = wavFilesNames.begin(); it != wavFilesNames.end(); it++) {
		wololo::DrsFileInfo wav;
		size_t size = fs::file_size(inputDir + *it + ".wav");
		wav.file_id = stoi(*it);
		wav.file_data_offset = offset;
		wav.file_size = size;
		offset += size;
		wavFiles.push_back(wav);
	}

	// header infos

	wololo::DrsHeader const header = {
		{ 'C', 'o', 'p', 'y', 'r',
		  'i', 'g', 'h', 't', ' ',
		  '(', 'c', ')', ' ', '1',
		  '9', '9', '7', ' ', 'E',
		  'n', 's', 'e', 'm', 'b',
		  'l', 'e', ' ', 'S', 't',
		  'u', 'd', 'i', 'o', 's',
		  '.', '\x1a' }, // copyright
		{ '1', '.', '0', '0' }, // version
		{ 't', 'r', 'i', 'b', 'e' }, // ftype
		numberOfTables, // table_count
		offsetOfFirstFile // file_offset
	};

	// table infos

	wololo::DrsTableInfo const slpTableInfo = {
		0x20, // file_type, MAGIC
		{ 'p', 'l', 's' }, // file_extension, "slp" in reverse
		sizeof (wololo::DrsHeader) + sizeof (wololo::DrsFileInfo) * numberOfTables, // file_info_offset
		(int) slpFiles.size() // num_files
	};
	wololo::DrsTableInfo const wavTableInfo = {
		0x20, // file_type, MAGIC
		{ 'v', 'a', 'w' }, // file_extension, "wav" in reverse
		(int) (sizeof (wololo::DrsHeader) +  sizeof (wololo::DrsFileInfo) * numberOfTables + sizeof (wololo::DrsFileInfo) * slpFiles.size()), // file_info_offset
		(int) wavFiles.size() // num_files
	};


	this->ui->label->setText((translation["working"]+"\n"+translation["workingDrs2"]).c_str());
	this->ui->label->repaint();
	// now write the actual drs file

	// header
	out->write(header.copyright, sizeof (wololo::DrsHeader::copyright));
	out->write(header.version, sizeof (wololo::DrsHeader::version));
	out->write(header.ftype, sizeof (wololo::DrsHeader::ftype));
	out->write(reinterpret_cast<const char *>(&header.table_count), sizeof (wololo::DrsHeader::table_count));
	out->write(reinterpret_cast<const char *>(&header.file_offset), sizeof (wololo::DrsHeader::file_offset));

	// table infos
	out->write(reinterpret_cast<const char *>(&slpTableInfo.file_type), sizeof (wololo::DrsTableInfo::file_type));
	out->write(slpTableInfo.file_extension, sizeof (wololo::DrsTableInfo::file_extension));
	out->write(reinterpret_cast<const char *>(&slpTableInfo.file_info_offset), sizeof (wololo::DrsTableInfo::file_info_offset));
	out->write(reinterpret_cast<const char *>(&slpTableInfo.num_files), sizeof (wololo::DrsTableInfo::num_files));

	out->write(reinterpret_cast<const char *>(&wavTableInfo.file_type), sizeof (wololo::DrsTableInfo::file_type));
	out->write(wavTableInfo.file_extension, sizeof (wololo::DrsTableInfo::file_extension));
	out->write(reinterpret_cast<const char *>(&wavTableInfo.file_info_offset), sizeof (wololo::DrsTableInfo::file_info_offset));
	out->write(reinterpret_cast<const char *>(&wavTableInfo.num_files), sizeof (wololo::DrsTableInfo::num_files));

	// file infos
	for (std::vector<wololo::DrsFileInfo>::iterator it = slpFiles.begin(); it != slpFiles.end(); it++) {
		out->write(reinterpret_cast<const char *>(&it->file_id), sizeof (wololo::DrsFileInfo::file_id));
		out->write(reinterpret_cast<const char *>(&it->file_data_offset), sizeof (wololo::DrsFileInfo::file_data_offset));
		out->write(reinterpret_cast<const char *>(&it->file_size), sizeof (wololo::DrsFileInfo::file_size));
	}

	for (std::vector<wololo::DrsFileInfo>::iterator it = wavFiles.begin(); it != wavFiles.end(); it++) {
		out->write(reinterpret_cast<const char *>(&it->file_id), sizeof (wololo::DrsFileInfo::file_id));
		out->write(reinterpret_cast<const char *>(&it->file_data_offset), sizeof (wololo::DrsFileInfo::file_data_offset));
		out->write(reinterpret_cast<const char *>(&it->file_size), sizeof (wololo::DrsFileInfo::file_size));
	}


	this->ui->label->setText((translation["working"]+"\n"+translation["workingDrs3"]).c_str());
	this->ui->label->repaint();
	// now write the actual files
	modIt = moddedFilesNames.begin();
	for (std::vector<std::string>::iterator it = slpFilesNames.begin(); it != slpFilesNames.end();) {
		int comp = (modIt != moddedFilesNames.end())?(*modIt).compare(*it):1;
		if(comp == 0) {
			std::ifstream srcStream = std::ifstream(moddedInputDir + *modIt + ".slp", std::ios::binary);
			*out << srcStream.rdbuf();
			it++;
			modIt++;
		} else if (comp < 0) {
			std::ifstream srcStream = std::ifstream(moddedInputDir + *modIt + ".slp", std::ios::binary);
			*out << srcStream.rdbuf();
			modIt++;
		} else {
			std::ifstream srcStream = std::ifstream(inputDir + *it + ".slp", std::ios::binary);
			*out << srcStream.rdbuf();
			it++;
		}

	}

	for (std::vector<std::string>::iterator it = wavFilesNames.begin(); it != wavFilesNames.end(); it++) {
		std::ifstream srcStream(inputDir + *it + ".wav", std::ios::binary);
		*out << srcStream.rdbuf();
	}
}

void MainWindow::uglyHudHack(std::string const inputDir, std::string const moddedDir) {
	/*
	 * We have more than 30 civs, so we need to space the interface files further apart
	 * This adds +10 for each gap between different file types
	 */
	this->ui->label->repaint();
	int const HudFiles[] = {51100, 51130, 51160};
	for (size_t baseIndex = 0; baseIndex < sizeof HudFiles / sizeof (int); baseIndex++) {
		for (int i = 1; i < 24; i++) {
			std::string dst = moddedDir + std::to_string(HudFiles[baseIndex]+i+baseIndex*10) + ".slp";
			std::string src = inputDir + std::to_string(HudFiles[baseIndex]+i) + ".slp";
			if(fs::exists(src))
				fs::copy_file(src, dst);
		}
	}

	/*
	 * copies the Slav hud files for AK civs, the good way of doing this would be to extract
	 * the actual AK civs hud files from
	 * Age2HD\resources\_common\slp\game_b[24-27].slp correctly, but I haven't found a way yet
	 */
	int const slavHudFiles[] = {51123, 51163, 51203};
	for (size_t baseIndex = 0; baseIndex < sizeof slavHudFiles / sizeof (int); baseIndex++) {
		for (size_t i = 1; i <= 8; i++) {
			std::string dst = moddedDir + std::to_string(slavHudFiles[baseIndex]+i) + ".slp";
			std::string src = moddedDir + std::to_string(slavHudFiles[baseIndex]) + ".slp";
			fs::copy_file(src, dst);
		}
	}
}

void MainWindow::copyCivIntroSounds(fs::path inputDir, fs::path outputDir) {
	std::string const civs[] = {"italians", "indians", "incas", "magyars", "slavs",
								"portuguese", "ethiopians", "malians", "berbers", "burmese", "malay", "vietnamese", "khmer"};
	for (size_t i = 0; i < sizeof civs / sizeof (std::string); i++) {
		boost::system::error_code ec;
		fs::copy_file(inputDir / (civs[i] + ".mp3"), outputDir / (civs[i] + ".mp3"), ec);
	}
}

std::string MainWindow::tolower(std::string line) {
	std::transform(line.begin(), line.end(), line.begin(), static_cast<int(*)(int)>(std::tolower));
	return line;
}

void MainWindow::createMusicPlaylist(std::string inputDir, std::string const outputDir) {
	boost::replace_all(inputDir, "/", "\\");
	std::ofstream outputFile(outputDir);
	for (int i = 1; i <= 23; i++ ) {
		outputFile << inputDir << "xmusic" << std::to_string(i) << ".mp3" <<  std::endl;
	}
}

void MainWindow::copyHDMaps(fs::path inputDir, fs::path outputDir) {

	fs::path moddedAssetsPath = fs::path("map_temp/");

	const std::set<std::string> exclude = {
		"Arabia",
		"Archipelago",
		"Arena",
		"Baltic",
		"Black_Forest",
		"Blind_Random",
		"Coastal",
		"Continental",
		"Crater_Lake",
		"Fortress",
		"Ghost_Lake",
		"Gold_Rush",
		"Highland",
		"Islands",
		"Mediterranean",
		"Migration",
		"Mongolia",
		"nomad",
		"Oasis",
		"Rivers",
		"Salt_Marsh",
		"Scandinavia",
		"Team_Islands",
		"Yucatan"
	};

	std::vector<fs::path> mapNames;
	std::vector<fs::path> mapNamesSorted;
	std::vector<fs::path> existingMapNames;
	for (fs::directory_iterator end, it(inputDir); it != end; it++) {
		std::string stem = it->path().stem().string();
		if (exclude.find(stem) == exclude.end()) {
			std::string extension = it->path().extension().string();
			if ((extension == ".rms" || extension == ".rms2") && stem.substr(0,10) != "real_world" && stem.substr(0,11) != "special_map" && stem.substr(0,3) != "CtR") {
				mapNames.push_back(*it);
			}
		}
	}
	for (fs::directory_iterator end, it(outputDir); it != end; it++) {
		std::string filename = it->path().filename().string();
		if (filename.substr(0,3) == "ZR@")
			existingMapNames.push_back(it->path().parent_path()/filename.substr(3,std::string::npos));
		else
			existingMapNames.push_back(it->path());
	}
	sort(existingMapNames.begin(), existingMapNames.end());
	sort(mapNames.begin(), mapNames.end());
	std::vector<fs::path>::iterator modIt = existingMapNames.begin();
	for (std::vector<fs::path>::iterator it = mapNames.begin(); it != mapNames.end();) {
		int comp = modIt != existingMapNames.end()?(modIt->stem().string()).compare(it->stem().string()):1;
		if(comp == 0) {
			it++;
			modIt++;
		} else if (comp < 0) {
			modIt++;
		} else {
			mapNamesSorted.push_back(*it);
			it++;
		}
	}
	std::set<fs::path> terrainOverrides;
	std::vector<std::tuple<std::string,std::string,std::string,std::string,std::string,std::string,bool,std::string,std::string>> replacements = {
		//<Name,Regex Pattern if needed,replace name,terrain ID, replace terrain ID,slp to replace,upgrade trees?,tree to replace,new tree>
		std::make_tuple("DRAGONFOREST","DRAGONFORES(T?)","DRAGONFORES$1","48","21","15029.slp",true,"SNOWPINETREE","DRAGONTREE"),
		std::make_tuple("ACACIA_FOREST","AC(C?)ACIA(_?)FORES(T?)","AC$1ACIA$2FORES$3","50","13","15010.slp",true,"PALMTREE","ACACIA_TREEE"),
		std::make_tuple("DLC_RAINFOREST","","DLC_RAINFOREST","56","10","15011.slp",true,"FOREST_TREE","DLC_RAINTREE"),
		std::make_tuple("BAOBAB","","BAOBAB","49","16","",false,"",""),
		std::make_tuple("DLC_MANGROVESHALLOW","","DLC_MANGROVESHALLOW","54","41","15030.slp",false,"",""),
		std::make_tuple("DLC_MANGROVEFOREST","","DLC_MANGROVEFOREST","55","26","15030.slp",false,"",""),
		std::make_tuple("DLC_NEWSHALLOW","","DLC_NEWSHALLOW","59","4","15014.slp",false,"",""),
		std::make_tuple("SAVANNAH","","SAVANNAH","41","14","15010.slp",false,"",""),
		std::make_tuple("DIRT4","((DLC_)?)DIRT4","$1DIRT4","42","3","15007.slp",false,"",""),
		std::make_tuple("MOORLAND","","MOORLAND","44","9","15009.slp",false,"",""),
		std::make_tuple("CRACKEDIT","","CRACKEDIT","45","15","15000.slp",false,"",""),
		std::make_tuple("QUICKSAND","","QUICKSAND","46","40","15018.slp",false,"",""),
		std::make_tuple("BLACK","","BLACK","47","40","15018.slp",false,"",""),
		std::make_tuple("DLC_ROCK","","DLC_ROCK","40","40","15018.slp",false,"",""),
		std::make_tuple("DLC_BEACH2","","DLC_BEACH2","51","2","15017.slp",false,"",""),
		std::make_tuple("DLC_BEACH3","","DLC_BEACH3","52","2","15017.slp",false,"",""),
		std::make_tuple("DLC_BEACH4","","DLC_BEACH4","53","2","15017.slp",false,"",""),
		std::make_tuple("DLC_DRYROAD","","DLC_DRYROAD","43","25","15019.slp",false,"",""),
		std::make_tuple("DLC_WATER4","","DLC_WATER4","57","22","15015.slp",false,"",""),
		std::make_tuple("DLC_WATER5","","DLC_WATER5","58","1","15002.slp",false,"",""),
		std::make_tuple("DLC_DRYROAD","","DLC_DRYROAD","43","25","15019.slp",false,"",""),
		std::make_tuple("DLC_JUNGLELEAVES","","DLC_JUNGLELEAVES","62","11","15006.slp",false,"",""),
		std::make_tuple("DLC_JUNGLEROAD","","DLC_JUNGLEROAD","62","39","15031.slp",false,"",""),
		std::make_tuple("DLC_JUNGLEGRASS","","DLC_JUNGLEGRASS","61","12","15008.slp",false,"","")
	};
	for (std::vector<fs::path>::iterator it = mapNamesSorted.begin(); it != mapNamesSorted.end(); it++) {
		std::ifstream input(inputDir.string()+it->filename().string());
		std::string str(static_cast<std::stringstream const&>(std::stringstream() << input.rdbuf()).str());
		for (std::vector<std::tuple<std::string,std::string,std::string,std::string,std::string,std::string,bool,std::string,std::string>>::iterator repIt = replacements.begin(); repIt != replacements.end(); repIt++) {
			if((std::get<0>(*repIt)=="DLC_WATER4"||std::get<0>(*repIt)=="DLC_WATER5") && (str.find("MED_WATER")!=std::string::npos || str.find("DEEP_WATER")!=std::string::npos)) {
				boost::replace_all(str, "#const "+std::get<0>(*repIt)+" "+std::get<3>(*repIt), "#const "+std::get<0>(*repIt)+" "+std::get<4>(*repIt));
				continue;
			}
			std::regex terrainConstDef;
			std::regex terrainName;
			if(std::get<1>(*repIt)=="") {
				terrainConstDef = std::regex("#const\\s+" +std::get<0>(*repIt)+ "\\s+" +std::get<3>(*repIt));
				terrainName = std::regex(std::get<0>(*repIt));
			} else {
				terrainConstDef = std::regex("#const\\s+" +std::get<1>(*repIt)+ "\\s+" +std::get<3>(*repIt));
				terrainName = std::regex(std::get<1>(*repIt));
			}
			if(std::regex_search(str,terrainName)) {
				str = std::regex_replace(str,terrainConstDef, "#const "+std::get<2>(*repIt)+" "+std::get<4>(*repIt));
				if(std::get<5>(*repIt) != "") {
					fs::copy_file(moddedAssetsPath/(std::get<0>(*repIt)+".slp"),moddedAssetsPath/std::get<5>(*repIt),fs::copy_option::overwrite_if_exists);
					terrainOverrides.insert(moddedAssetsPath/std::get<5>(*repIt));
					if(std::get<6>(*repIt)) {
						if(str.find("<PLAYER_SETUP>")!=std::string::npos)
							str = std::regex_replace(str, std::regex("<PLAYER_SETUP>\\s*(\\r*)\\n"),
								"<PLAYER_SETUP>$1\n  effect_amount GAIA_UPGRADE_UNIT "+std::get<7>(*repIt)+" "+std::get<8>(*repIt)+" 0$1\n");
						else
							str = std::regex_replace(str, std::regex("#include_drs\\s+random_map\\.def\\s*(\\r*)\\n"),
								"#include_drs random_map.def$1\n<PLAYER_SETUP>\n  effect_amount GAIA_UPGRADE_UNIT "+std::get<7>(*repIt)+" "+std::get<8>(*repIt)+" 0$1\n");
					}
				}
			}
		}
		if(str.find("DLC_MANGROVESHALLOW")!=std::string::npos) {
			terrainOverrides.insert(moddedAssetsPath/"15004.slp");
			terrainOverrides.insert(moddedAssetsPath/"15005.slp");
			terrainOverrides.insert(moddedAssetsPath/"15021.slp");
			terrainOverrides.insert(moddedAssetsPath/"15022.slp");
			terrainOverrides.insert(moddedAssetsPath/"15023.slp");
		}
		str = regex_replace(str, std::regex("#const\\s+BAOBAB\\s+49"), "#const BAOBAB 16");

		std::string mapName = it->stem().string()+".rms";
		std::ofstream out(outputDir.string()+"/"+mapName);
		out << str;
		out.close();
		if (mapName.substr(0,3) == "rw_" || mapName.substr(0,3) == "sm_") {
			terrainOverrides.insert(fs::path(inputDir.string()+"/"+it->stem().string()+".scx"));
		}
		if (terrainOverrides.size() != 0) {
			QuaZip zip(QString((outputDir.string()+"/ZR@"+mapName).c_str()));
			zip.open(QuaZip::mdAdd, NULL);
			terrainOverrides.insert(fs::path(outputDir.string()+"/"+mapName));
			for(std::set<fs::path>::iterator files = terrainOverrides.begin(); files != terrainOverrides.end(); files++) {
				QuaZipFile outFile(&zip);
				QuaZipNewInfo fileInfo(QString((*files).filename().string().c_str()));
				fileInfo.uncompressedSize = fs::file_size((*files));
				outFile.open(QIODevice::WriteOnly,fileInfo,NULL,0,0,0,false);
				QFile inFile;
				inFile.setFileName((*files).string().c_str());
				inFile.open(QIODevice::ReadOnly);
				copyData(inFile, outFile);
				outFile.close();
				inFile.close();
			}
			zip.close();
			fs::remove(fs::path(outputDir.string()+"/"+mapName));
		}
		terrainOverrides.clear();
	}
}

void MainWindow::transferHdDatElements(genie::DatFile *hdDat, genie::DatFile *aocDat) {

	aocDat->Sounds = hdDat->Sounds;
	aocDat->GraphicPointers = hdDat->GraphicPointers;
	aocDat->Graphics = hdDat->Graphics;
	aocDat->Techages = hdDat->Techages;
	aocDat->UnitHeaders = hdDat->UnitHeaders;
	aocDat->Civs = hdDat->Civs;
	aocDat->Researchs = hdDat->Researchs;
	aocDat->UnitLines = hdDat->UnitLines;
	aocDat->TechTree = hdDat->TechTree;

	//Copy Terrains
	aocDat->TerrainBlock.TerrainsUsed2 = 42;
	aocDat->TerrainsUsed1 = 42;
	terrainSwap(hdDat, aocDat, 41,54,15030); //mangrove terrain
	terrainSwap(hdDat, aocDat, 26,55,15030); //mangrove forest
	terrainSwap(hdDat, aocDat, 16,49,15000); //baobab forest
	aocDat->TerrainBlock.Terrains[35].TerrainToDraw = -1;
	aocDat->TerrainBlock.Terrains[35].SLP = 15024;
	aocDat->TerrainBlock.Terrains[35].Name2 = "g_ice";

	terrainSwap(hdDat, aocDat, 15,45,15000); //cracked sand
}

void MainWindow::terrainSwap(genie::DatFile *hdDat, genie::DatFile *aocDat, int tNew, int tOld, int slpID) {
	aocDat->TerrainBlock.Terrains[tNew] = hdDat->TerrainBlock.Terrains[tOld];
	aocDat->TerrainBlock.Terrains[tNew].SLP = slpID;
	if (tNew == 41) {
		for(size_t j = 0; j < aocDat->TerrainRestrictions.size(); j++) {
			aocDat->TerrainRestrictions[j].PassableBuildableDmgMultiplier.push_back(hdDat->TerrainRestrictions[j].PassableBuildableDmgMultiplier[tOld]);
			aocDat->TerrainRestrictions[j].TerrainPassGraphics.push_back(hdDat->TerrainRestrictions[j].TerrainPassGraphics[tOld]);
			if (j == 4)
				aocDat->TerrainRestrictions[j].PassableBuildableDmgMultiplier[tNew] = 1.2;
		}
	} else {
		for(size_t j = 0; j < aocDat->TerrainRestrictions.size(); j++) {
			aocDat->TerrainRestrictions[j].PassableBuildableDmgMultiplier[tNew] = hdDat->TerrainRestrictions[j].PassableBuildableDmgMultiplier[tOld];
			aocDat->TerrainRestrictions[j].TerrainPassGraphics[tNew] = hdDat->TerrainRestrictions[j].TerrainPassGraphics[tOld];
		}
	}
}

void MainWindow::hotkeySetup() {

	fs::path nfz1Path("resources/Player1.nfz");
	fs::path nfzPath = outPath / "player.nfz";
	fs::path aocHkiPath("resources/player1.hki");
	fs::path customHkiPath("player1.hki");
	fs::path hkiPath = HDPath / ("Profiles/player0.hki");
	fs::path hkiOutPath = outPath / "player1.hki";
	fs::path hki2OutPath = outPath / "player2.hki";
	fs::path nfzOutPath = outPath / "Voobly Mods/AOC/Data Mods/WololoKingdoms/Player.nfz";

	fs::remove(nfzOutPath);
	fs::remove(nfzUpOutPath);
	if(fs::exists(nfzPath)) //Copy the Aoc Profile
		fs::copy_file(nfzPath, nfzOutPath);
	else //otherwise copy the default profile included
		fs::copy_file(nfz1Path, nfzOutPath);
	if(this->ui->createExe->isChecked()) { //Profiles for UP
		if(fs::exists(nfzPath)) //Copy the Aoc Profile
			fs::copy_file(nfzPath,nfzUpOutPath);
		else //otherwise copy the default profile included
			fs::copy_file(nfz1Path,nfzUpOutPath);
	}
	//Copy hotkey files
	if(fs::exists(customHkiPath)) {
		fs::copy_file(customHkiPath, modHkiOutPath,fs::copy_option::overwrite_if_exists);
		if(fs::exists(hki2OutPath))
			fs::copy_file(customHkiPath, modHki2OutPath,fs::copy_option::overwrite_if_exists);
		if(this->ui->createExe->isChecked()) {
			fs::copy_file(customHkiPath, upHkiOutPath,fs::copy_option::overwrite_if_exists);
			if(fs::exists(hki2OutPath))
				fs::copy_file(customHkiPath, upHki2OutPath,fs::copy_option::overwrite_if_exists);
		}
		return;
	}
	if (this->ui->hotkeyChoice->currentIndex() == 1 && !fs::exists(hkiOutPath))
		fs::copy_file(aocHkiPath, hkiOutPath);
	if (this->ui->hotkeyChoice->currentIndex() == 2) {
		fs::copy_file(hkiPath, modHkiOutPath,fs::copy_option::overwrite_if_exists);
		if(fs::exists(hki2OutPath))
			fs::copy_file(hkiPath, modHki2OutPath,fs::copy_option::overwrite_if_exists);
		if(this->ui->createExe->isChecked()) {
			fs::copy_file(hkiPath, upHkiOutPath,fs::copy_option::overwrite_if_exists);
			if(fs::exists(hki2OutPath))
				fs::copy_file(hkiPath, upHki2OutPath,fs::copy_option::overwrite_if_exists);
		}
	}
	if(this->ui->hotkeyChoice->currentIndex() == 3) {
		fs::path backup = hkiOutPath;
		backup+=".bak";
		if(fs::exists(hkiOutPath))
			fs::copy_file(hkiOutPath, backup,fs::copy_option::overwrite_if_exists);
		fs::copy_file(hkiPath, hkiOutPath,fs::copy_option::overwrite_if_exists);
		if(fs::exists(hki2OutPath)) {
			backup = hki2OutPath;
			backup+=".bak";
			fs::copy_file(hki2OutPath, backup,fs::copy_option::overwrite_if_exists);
			fs::copy_file(hkiPath, hki2OutPath,fs::copy_option::overwrite_if_exists);
		}
	}
}

int MainWindow::run()
{
	this->setEnabled(false);
	this->ui->label->setText(translation["working"].c_str());
	this->ui->label->repaint();
	qApp->processEvents();
	QDialog* dialog;
	int ret = 0;

	try {
		fs::path keyValuesStringsPath = HDPath / "resources/" / language / "/strings/key-value/key-value-strings-utf8.txt";
		fs::path vooblyDir = outPath / "Voobly Mods/AOC/Data Mods/WololoKingdoms/";
		std::string aocDatPath = HDPath.string() + "resources/_common/dat/empires2_x1_p1.dat";
		std::string hdDatPath = HDPath.string() + "resources/_common/dat/empires2_x2_p1.dat";
		fs::path modLangPath("resources/language.ini");
		fs::path languageIniPath = vooblyDir / "language.ini";
		std::string versionIniPath = vooblyDir.string() + "version.ini";
		fs::path soundsInputPath = HDPath / "resources/_common/sound/";
		fs::path soundsOutputPath = vooblyDir / "Sound/";
		fs::path tauntInputPath = HDPath / "resources/en/sound/taunt/";
		fs::path tauntOutputPath = vooblyDir / "Taunt/";
		fs::path xmlPath("resources/WK.xml");
		fs::path xmlOutPath = vooblyDir / "age2_x1.xml";
		fs::path langDllFile("language_x1_p1.dll");
		fs::path langDllPath = langDllFile;
		fs::path xmlOutPathUP = outPath / "Games/WK.xml";
		fs::path aiInputPath("resources/Script.Ai");
		std::string drsOutPath = vooblyDir.string() + "Data/gamedata_x1_p1.drs";
		fs::path assetsPath = HDPath / "resources/_common/drs/gamedata_x2/";
		fs::path moddedAssetsPath("assets/");
		fs::path outputDatPath = vooblyDir / "Data/empires2_x1_p1.dat";
		fs::path upDir = outPath / "Games/WololoKingdoms/";
		std::string const UPModdedExe = "WK";
		fs::path UPExe("resources/SetupAoc.exe");
		fs::path UPExeOut = outPath / "SetupAoc.exe";
		fs::path pwInputDir("resources/pussywood");
		fs::path gridInputDir("resources/Grid No Snow");
		fs::path newTerrainInputDir("resources/terrains");
		fs::path newGridTerrainInputDir("resources/new grid terrains");
		fs::path tempMapDir("map_temp/");
		fs::path gridNoSnowInputDir("resources/Grid");
		fs::path noSnowInputDir("resources/No Snow");
		fs::path wallsInputDir("resources/short_walls");
		fs::path gamedata_x1("resources/gamedata_x1.drs");

		std::string line;
		
		fs::remove_all(moddedAssetsPath);
		fs::remove_all(tempMapDir);
		fs::create_directories(moddedAssetsPath);
		fs::create_directories(tempMapDir);

		if(this->ui->usePw->isChecked() || this->ui->useGrid->isChecked() || this->ui->useWalls->isChecked()) {
			this->ui->label->setText((translation["working"]+"\n"+translation["workingMods"]).c_str());
			this->ui->label->repaint();
		}
		if(this->ui->usePw->isChecked())
			recCopy(pwInputDir, moddedAssetsPath);
		if(this->ui->useGrid->isChecked()) {
			recCopy(gridInputDir, moddedAssetsPath);
			recCopy(newGridTerrainInputDir,tempMapDir);
			if(this->ui->useNoSnow->isChecked())
				recCopy(gridNoSnowInputDir, moddedAssetsPath, false, true);
		} else {
			recCopy(newTerrainInputDir,tempMapDir);
			if(this->ui->useNoSnow->isChecked())
				recCopy(noSnowInputDir, moddedAssetsPath);
		}
		if(this->ui->useWalls->isChecked())
			recCopy(wallsInputDir, moddedAssetsPath);
	
		fs::remove_all(vooblyDir/"Data");
		fs::remove_all(vooblyDir/"Script.Ai/Brutal");
		fs::remove(vooblyDir/"Script.Ai/BruteForce.ai");
		fs::remove(vooblyDir/"Script.Ai/BruteForce.per");
		fs::remove(vooblyDir/"age2_x1.xml");
		fs::remove(vooblyDir/"language.ini");
		fs::remove(vooblyDir/"version.ini");
		fs::remove_all(upDir/"Data");
		fs::remove_all(upDir/"Script.Ai/Brutal");
		fs::remove(upDir/"Script.Ai/BruteForce.ai");
		fs::remove(upDir/"Script.Ai/BruteForce.per");
		fs::remove(outPath/"Games/WK.xml");
		fs::create_directories(vooblyDir/"Data");
		fs::create_directories(vooblyDir/"Sound/stream");
		fs::create_directories(vooblyDir/"Taunt");
		fs::create_directories(upDir);

		this->ui->label->setText((translation["working"]+"\n"+translation["workingFiles"]).c_str());
		this->ui->label->repaint();
		std::ofstream versionOut(versionIniPath);
		versionOut << version << std::endl;

		boolean aocFound = outPath != HDPath/"WololoKingdoms/out/";
		copyCivIntroSounds(soundsInputPath / "civ/", soundsOutputPath / "stream/");
		createMusicPlaylist(soundsInputPath.string() + "music/", soundsOutputPath.string() + "music.m3u");
		recCopy(tauntInputPath, tauntOutputPath, true);
		fs::copy_file(xmlPath, xmlOutPath);
		if (aocFound) {
			recCopy(outPath/"Random", vooblyDir/"Script.Rm", true);
		}
		copyHDMaps(assetsPath, vooblyDir/"Script.Rm");
		copyHDMaps(HDPath/"resources/_common/random-map-scripts/", vooblyDir/"Script.Rm");
		if(this->ui->copyMaps->isChecked())
			copyHDMaps("resources/Script.Rm/", vooblyDir/"Script.Rm");

		//If wanted, the BruteForce AI could be included as a "standard" AI.
		recCopy(aiInputPath, vooblyDir/"Script.Ai", true);

		if(this->ui->createExe->isChecked()) {
			fs::create_directories(upDir / "Data");
			recCopy(vooblyDir / "Sound", upDir / "Sound", true);
			recCopy(vooblyDir / "Taunt", upDir / "Taunt", true);
			fs::copy_file(xmlPath, xmlOutPathUP);
			recCopy(vooblyDir / "Script.Rm", upDir / "Script.Rm", true);
			recCopy(vooblyDir / "Script.Ai", upDir / "Script.Ai", true);
		}
		if(this->ui->hotkeyChoice->currentIndex() != 0 || fs::exists("player1.hki"))
			hotkeySetup();
		recCopy(gamedata_x1, vooblyDir/"Data/gamedata_x1.drs", false);




		this->ui->label->setText((translation["working"]+"\n"+translation["workingAoc"]).c_str());
		this->ui->label->repaint();

		genie::DatFile aocDat;
		aocDat.setVerboseMode(true);
		aocDat.setGameVersion(genie::GameVersion::GV_TC);
		aocDat.load(aocDatPath.c_str());

		this->ui->label->setText((translation["working"]+"\n"+translation["workingHD"]).c_str());
		this->ui->label->repaint();
		genie::DatFile hdDat;
		hdDat.setVerboseMode(true);
		hdDat.setGameVersion(genie::GameVersion::GV_Cysion);
		hdDat.load(hdDatPath.c_str());

		std::ofstream drsOut(drsOutPath, std::ios::binary);

		this->ui->label->setText((translation["working"]+"\n"+translation["workingInterface"]).c_str());
		this->ui->label->repaint();
		uglyHudHack(assetsPath.string(),moddedAssetsPath.string());
		makeDrs(assetsPath.string(), moddedAssetsPath.string(), &drsOut);

		fs::remove_all(moddedAssetsPath);
		fs::remove_all(tempMapDir);

		this->ui->label->setText((translation["working"]+"\n"+translation["workingDat"]).c_str());
		this->ui->label->repaint();
		transferHdDatElements(&hdDat, &aocDat);

		wololo::DatPatch patchTab[] = {

			wololo::berbersUTFix,
			wololo::vietFix,
			wololo::malayFix,
			//			wololo::demoShipFix,
			wololo::ethiopiansFreePikeUpgradeFix,
			wololo::hotkeysFix,
			wololo::maliansFreeMiningUpgradeFix,
			wololo::portugueseFix,
			wololo::disableNonWorkingUnits,
			wololo::feitoriaFix,
			wololo::burmeseFix,
			wololo::incaFix,
			wololo::smallFixes,
			wololo::ai900UnitIdFix
		};


		std::map<int, std::string> langReplacement;
		//Fix errors in civ descriptions
		if (language == "en") {
			langReplacement[20162] = "Infantry civilization \\n\\n· Infantry move 15% faster \\n· Lumberjacks work 15% faster \\n· Siege weapons fire 20% faster \\n· Can convert sheep even if enemy units are next to them. \\n\\n<b>Unique Unit:<b> Woad Raider (infantry) \\n\\n<b>Unique Techs:<b> Stronghold (Castles and towers fire 20% faster); Furor Celtica (Siege Workshop units have +40% HP)\\n\\n<b>Team Bonus:<b> Siege Workshops work 20% faster";
			langReplacement[20166] = "Cavalry civilization \\n\\n· Do not need houses, but start with -100 wood \\n· Cavalry Archers cost -10% Castle, -20% Imperial Age \\n· Trebuchets +35% accuracy against units \\n\\n<b>Unique Unit:<b> Tarkan (cavalry) \\n\\n<b>Unique Techs:<b> Marauders (Create Tarkans at stables); Atheism (+100 years Relic, Wonder victories; Spies/Treason costs -50%)\\n\\n<b>Team Bonus:<b> Stables work 20% faster";
			langReplacement[20170] = "Infantry civilization \\n\\n· Start with a free llama \\n· Villagers affected by Blacksmith upgrades \\n· Houses support 10 population \\n· Buildings cost -15% stone\\n\\n<b>Unique Units:<b> Kamayuk (infantry), Slinger (archer)\\n\\n<b>Unique Techs:<b> Andean Sling (Skirmishers and Slingers no minimum range); Couriers (Kamayuks, Slingers, Eagles +1 armor/+2 pierce armor)\\n\\n<b>Team Bonus:<b> Farms built 2x faster";
			langReplacement[20165] = "Archer civilization \\n\\n· Start with +1 villager, but -50 food \\n· Resources last 15% longer \\n· Archers cost -10% Feudal, -20% Castle, -30% Imperial Age \\n\\n<b>Unique Unit:<b> Plumed Archer (archer) \\n\\n<b>Unique Techs:<b> Obsidian Arrows (Archers, Crossbowmen and Arbalests +12 attack vs. Towers/Stone Walls, +6 attack vs. other buildings); El Dorado (Eagle Warriors have +40 hit points)\\n\\n<b>Team Bonus:<b> Walls cost -50%";
			langReplacement[20158] = "Camel and naval civilization \\n· Market trade cost only 5% \\n· Market costs -75 wood \\n· Transport Ships 2x hit points, \\n 2x carry capacity \\n· Galleys attack 20% faster \\n· Cavalry archers +4 attack vs. buildings \\n\\n<b>Unique Unit:<b> Mameluke (camel) \\n\\n<b>Unique Techs:<b> Madrasah (Killed monks return 33% of their cost); Zealotry (Camels, Mamelukes +30 hit points)\\n\\n<b>Team Bonus:<b> Foot archers +2 attack vs. buildings";
			langReplacement[20163] = "Gunpowder and Monk civilization \\n\\n· Builders work 30% faster \\n· Blacksmith upgrades don't cost gold \\n· Cannon Galleons fire faster and with Ballistics) \\n· Gunpowder units fire 15% faster\\n\\n<b>Unique Units:<b> Conquistador (mounted hand cannoneer), Missionary (mounted Monk) \\n\\n<b>Unique Techs:<b> Inquisition (Monks convert faster); Supremacy (villagers better in combat)\\n\\n<b>Team Bonus:<b> Trade units generate +25% gold";
			//Add that the Genitour and Imperial Skirmishers are Mercenary Units, since there is no other visual difference in the tech tree
			langReplacement[26137] = "Create <b> Genitour<b> (<cost>) \\nBerber mercenary unit, available when teamed with a Berber player. Mounted skirmisher. Effective against Archers.<i> Upgrades: speed, hit points (Stable); attack, range, armor (Blacksmith); attack, accuracy (University); accuracy, armor, to Elite Genitour 500F, 450W (Archery Range); creation speed (Castle); more resistant to Monks (Monastery).<i> \\n<hp> <attack> <armor> <piercearmor> <range>";
			langReplacement[26139] = "Create <b> Elite Genitour<b> (<cost>) \\nBerber mercenary unit, available when teamed with a Berber player. Stronger than Genitour.<i> Upgrades: speed, hit points (Stable); attack, range, armor (Blacksmith); attack, accuracy (University); accuracy, armor (Archery Range); creation speed (Castle); more resistant to Monks (Monastery).<i> \\n<hp> <attack> <armor> <piercearmor> <range>";
			langReplacement[26190] = "Create <b> Imperial Skirmisher<b> (<cost>) \\nVietnamese mercenary unit, available when teamed with a Vietnamese player. Stronger than Elite Skirmisher. Attack bonus vs. archers. <i> Upgrades: attack, range, armor (Blacksmith); attack, accuracy (University); accuracy (Archery Range); creation speed (Castle); more resistant to Monks (Monastery).<i> \\n<hp> <attack> <armor> <piercearmor> <range>";
			langReplacement[26419] = "Create <b> Imperial Camel<b> (<cost>) \nUnique Indian upgrade. Stronger than Heavy Camel. Attack bonus vs. cavalry. <i> Upgrades: attack, armor (Blacksmith); speed, hit points (Stable); creation speed (Castle); more resistant to Monks (Monastery).<i> \n<hp> <attack> <armor> <piercearmor> <range>";
		}

		this->ui->label->setText((translation["working"]+"\n"+translation["workingPatches"]).c_str());
		this->ui->label->repaint();

		for (size_t i = 0, nbPatches = sizeof patchTab / sizeof (wololo::DatPatch); i < nbPatches; i++) {			
			patchTab[i].patch(&aocDat, &langReplacement);
		}


		if(this->ui->replaceTooltips->isChecked()) {
			/*
			 * Load modded strings instead of normal HD strings into lang replacement
			 */
			std::ifstream modLang(modLangPath.string());
			while (std::getline(modLang, line)) {
				int spaceIdx = line.find('=');
				std::string number = line.substr(0, spaceIdx);
				int nb;
				try {
					nb = stoi(number);
				}
				catch (std::invalid_argument const & e){
					continue;
				}
				line = line.substr(spaceIdx + 1, std::string::npos);

				std::wstring outputLine;
				ConvertCP2Unicode(line.c_str(), outputLine, CP_ACP);
				boost::replace_all(line, "\n", "\\n");
				line = wstrtostr(outputLine);
				langReplacement[nb] = line;
			}
		}


		std::ifstream langIn(keyValuesStringsPath.string());
		std::ofstream langOut(languageIniPath.string());
		genie::LangFile langDll;

		bool patchLangDll;
		if(this->ui->createExe->isChecked()) {
			if(!aocFound)
				patchLangDll = fs::exists(langDllPath);
			else {
				langDllPath = outPath / langDllPath;
				patchLangDll = fs::exists(langDllPath);
				if(patchLangDll)
				{
					if(fs::exists(langDllFile))
						fs::remove(langDllFile);
					fs::copy_file(langDllPath,langDllFile);
				}
			}
		} else {
			patchLangDll = false;
		}
		bool dllPatched = true;
		if (patchLangDll) {
			try {
				langDll.load((langDllFile.string()).c_str());
				langDll.setGameVersion(genie::GameVersion::GV_TC);
			} catch (const std::ifstream::failure& e) {
				//Try deleting and re-copying
				fs::remove(langDllFile);
				fs::copy_file(langDllPath,langDllFile);
				try {
					langDll.load((langDllFile.string()).c_str());
					langDll.setGameVersion(genie::GameVersion::GV_TC);
				} catch (const std::ifstream::failure& e) {
					fs::remove(langDllFile);
					if(!secondAttempt) {
						secondAttempt = true;
						ret = run();
						return ret;
					}
					dllPatched = false;
					patchLangDll = false;
				}
			}
		}
		else {
			if(this->ui->createExe->isChecked()) {
				line = translation["working"]+"\n"+translation["workingNoDll"];
				boost::replace_all(line,"<dll>",langDllPath.string());
				this->ui->label->setText(line.c_str());
				this->ui->label->repaint();
			}
		}
		convertLanguageFile(&langIn, &langOut, &langDll, patchLangDll, &langReplacement);
		if (patchLangDll) {
			try {
				line = translation["working"]+"\n"+translation["workingDll"];
				boost::replace_all(line,"<dll>",langDllFile.string());
				langDll.save();
				fs::copy_file(langDllFile,upDir/"data/"/langDllFile);
				fs::remove(langDllFile);
				this->ui->label->setText(line.c_str());
				this->ui->label->repaint();
			} catch (const std::ofstream::failure& e) {
				this->ui->label->setText(translation["workingError"].c_str());
				this->ui->label->repaint();
				try {
					langDll.save();
					fs::copy_file(langDllFile,upDir/"data/"/langDllFile);
					fs::remove(langDllFile);
					this->ui->label->setText(line.c_str());
					this->ui->label->repaint();
				} catch (const std::ofstream::failure& e) {
					if(!secondAttempt) {
						secondAttempt = true;
						ret = run();
						return ret;
					}
					dllPatched = false;
					patchLangDll = false;

				}
			}
		}

		aocDat.saveAs(outputDatPath.string().c_str());

		this->ui->label->setText((translation["working"]+"\n"+translation["workingUp"]).c_str());
		this->ui->label->repaint();



		if (aocFound) {

			if(this->ui->createExe->isChecked()) {
				recCopy(vooblyDir / "Data", upDir / "Data");
				if (!dllPatched) {
					dialog = new Dialog(this, translation["dialogNoDll"].c_str());
					dialog->exec();
				} else {
					if(fs::exists(UPExeOut)) {
						if(fs::file_size(UPExe) != fs::file_size((UPExeOut))) {
							fs::remove(UPExeOut);
							fs::copy_file(UPExe, UPExeOut);
						}
					} else {
						fs::copy_file(UPExe, UPExeOut);
					}
					system(("\""+UPExeOut.string()+"\" -g:"+UPModdedExe).c_str());
					line = translation["dialogExe"];
					boost::replace_all(line,"<exe>",UPModdedExe);
					dialog = new Dialog(this,line.c_str());
					dialog->exec();
				}
			} else {
				dialog = new Dialog(this,translation["dialogDone"].c_str());
				dialog->exec();
			}
			this->ui->label->setText(translation["workingDone"].c_str());
			if(fs::exists(outPath/"/compatslp")) {
				if(fs::exists(outPath/"/compatslp2"))
					fs::remove_all(outPath/"/compatslp2");
				recCopy(outPath/"/compatslp",outPath/"/compatslp2");
				fs::remove_all(outPath/"/compatslp");
				this->ui->label->setText(translation["workingCP"].c_str());
				if(this->ui->createExe->isChecked()) { //this causes a crash with UP 1.5 otherwise
					if(fs::file_size(outPath/"/data/blendomatic.dat") < 400000) {
						fs::rename(outPath/"/data/blendomatic.dat",outPath/"/data/blendomatic.dat.bak");
						fs::rename(outPath/"/data/blendomatic_x1.dat",outPath/"/data/blendomatic.dat");
					}
				}

			}
		} else {
			this->ui->label->setText(translation["workingNoAoc"].c_str());
			dialog = new Dialog(this,translation["dialogNoAoc"].c_str());
			dialog->exec();
		}
	}
	catch (std::exception const & e) {
		dialog = new Dialog(this,translation["dialogException"]+std::string()+e.what());
		dialog->exec();
		this->ui->label->setText(translation["error"].c_str());
		ret = 1;
	}
	catch (std::string const & e) {		
		dialog = new Dialog(this,translation["dialogException"]+e);
		dialog->exec();
		this->ui->label->setText(translation["error"].c_str());
		ret = 1;
	}

	this->setEnabled(true);
	this->ui->label->repaint();
	return ret;
}

bool MainWindow::copyData(QIODevice &inFile, QIODevice &outFile)
{
	while (!inFile.atEnd()) {
		char buf[4096];
		qint64 readLen = inFile.read(buf, 4096);
		if (readLen <= 0)
			return false;
		if (outFile.write(buf, readLen) != readLen)
			return false;
	}
	return true;
}

