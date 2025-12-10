#include "mainwindow.h"
#include "file_utils.hpp"
#include "gzip_it.hpp"
#include "time_strings.hpp"
#include "tinyjson.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <fstream>
#include <istream>
#include <ostream>

FileList clean_list(FileList list) {
    std::sort(list.begin(), list.end());
    FileList::iterator it = std::unique(list.begin(), list.end());
    list.resize(std::distance(list.begin(), it));
    return list;
}

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_mainwindow), timer_skip(false),
    isChanged(false) {
    ui->setupUi(this);

    connect(ui->pushButton_Load, &QPushButton::released, this,
            &mainwindow::btn_load_click);
    connect(ui->pushButton_Save, &QPushButton::released, this,
            &mainwindow::btn_save_click);
    connect(ui->pushButton_Add, &QPushButton::released, this,
            &mainwindow::btn_add_click);
    connect(ui->pushButton_Del, &QPushButton::released, this,
            &mainwindow::btn_del_click);
    connect(ui->pushButton_Clear, &QPushButton::released, this,
            &mainwindow::btn_clear_click);
    connect(ui->pushButton_Gen, &QPushButton::released, this,
            &mainwindow::btn_gen_click);

    connect(ui->pushButton_Const, &QPushButton::released, this,
            &mainwindow::btn_const_click);
    connect(ui->pushButton_zip, &QPushButton::released, this,
            &mainwindow::btn_zip_click);

    if (QApplication::arguments().count() > 1) {
        QString config = QApplication::arguments().at(1);
        load_config(config);
        isChanged = false;
    } else {
        QSettings mySettings("StestrupOld", "mainwindow");
        QString config = mySettings.value("config").toString();
        if (!config.isEmpty()) {
            load_config(config);
            isChanged = false;
        }
    }

    QTimer::singleShot(1000, this, &mainwindow::Timer1Timer);
}

mainwindow::~mainwindow() {
    QSettings mySetting;
    QSettings mySettings("StestrupOld", "mainwindow");
    mySettings.setValue("config", current_config);
}

void mainwindow::file_exit() { QApplication::quit(); }

void mainwindow::closeEvent(QCloseEvent *event) {
    if(isChanged)
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(
            this, "Setup is changed", tr("Save setup?\n"),
            QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
            QMessageBox::Yes);
        if (resBtn == QMessageBox::Cancel) {
            event->ignore();
        } else if (resBtn == QMessageBox::Yes) {
            btn_save_click();
            event->accept();
        }
    }
    else {
        event->accept();
    }
}


void mainwindow::setTitle() {
    QString title = "mainwindow  ";
    QString file = current_config;
    if (file.size() > 64)
        file = file.mid(0, 16) + "..." + file.mid(file.size() - 40);
    title += file;
    setWindowTitle(title);
    ui->listWidget_files->setToolTip(current_config);
    ui->listWidget_files->setToolTipDuration(2000);
}

void mainwindow::btn_load_click() {
    std::unique_ptr<QFileDialog> form{std::make_unique<QFileDialog>()};
    const QStringList filters({"File list (*.files)", "All files(*.*)"});
    form->setDirectory(QFileInfo(current_config).dir());
    if (form->exec()) {
        files.clear();
        FileList list;
        QStringList l = form->selectedFiles();
        for (size_t i = 0; i < l.count(); ++i)
            list.push_back(FileEntry{l[i]});
        clean_list(list);
        for (size_t i = 0; i < list.size(); ++i) {
            QString ext = extractFileExt(list[i].fileName);
            if (ext == "files")
                load_config(list[i].fileName, list);
        }
        for (int i = 0; i < list.size(); ++i) {
            QString ext = extractFileExt(list[i].fileName);
            if (ext != "files")
                files.push_back(list[i]);
        }
        clean_list(files);
        UpdateListBox();
        isChanged = false;
    }
}

void mainwindow::btn_save_click() {
    const QStringList filters({"File list (*.files)", "All files(*.*)"});
    QString dir;
    QString file;

    if (current_config != "") {
        dir = extractFileDir(current_config);
        file = extractFileName(current_config);
    } else {
        if (files.size() > 0) {
            dir = extractFileDir(files[0].fileName);
            file = "maketemplates.files";
        }
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), dir,
                                                        tr("File list (*.files)"));
        current_config = fileName;
    }
    setTitle();
    save_config();
    isChanged = false;
}

void mainwindow::btn_add_click() {
    std::unique_ptr<QFileDialog> form{std::make_unique<QFileDialog>()};
    form->setNameFilter(
        tr("Source files (*.html *.htm *.js *.css *.json *.svg *.ico "
           "*.files);;File list (*.files);;All files (* .*)"));
    form->setFileMode(QFileDialog::ExistingFiles);
    form->setDirectory(QFileInfo(current_config).dir());
    if (form->exec()) {
        FileList list = files;
        QStringList l = form->selectedFiles();
        for (size_t i = 0; i < l.count(); ++i)
            list.push_back({l[i]});
        for (size_t i = 0; i < list.size(); ++i) {
            QString ext = extractFileExt(list[i].fileName);
            if (ext == "files")
                load_config(list[i].fileName, list);
        }
        files.clear();
        for (int i = 0; i < list.size(); ++i) {
            QString ext = extractFileExt(list[i].fileName);
            if (ext != "files")
                files.push_back(list[i]);
        }
        files = clean_list(files);
        isChanged = true;
    }
    UpdateListBox();
}

void mainwindow::btn_del_click() {
    QList<QListWidgetItem *> items = ui->listWidget_files->selectedItems();
    foreach (QListWidgetItem *item, items) {
        // delete ui->listWidget_files->takeItem(ui->listWidget_files->row(item));
        FileList::iterator itr = files.begin();
        while (itr != files.end()) {
            QString fn = item->text();
            int p = fn.indexOf("(");
            if (p >= 0)
                fn = fn.left(p);
            fn = fn.trimmed();
            QString fn2 = itr->fileName;
            if (itr->fileName == fn) {
                files.erase(itr);
                itr = files.end();
            } else
                ++itr;
        }
    }
    UpdateListBox();
    isChanged = true;
}

void mainwindow::btn_clear_click() {
    ui->listWidget_files->clear();
    files.clear();
    current_config.clear();
    setTitle();
    UpdateListBox();
    isChanged = true;
}

void mainwindow::btn_gen_click() {
    if (files.size() == 0)
        return;

    QString path = QFileInfo(files[0].fileName).absolutePath();
    QDir().mkpath(path);

    QString cpp_name = path + "/" + ui->lineEdit_name->text() + ".cpp";
    QString hpp_name = path + "/" + ui->lineEdit_name->text() + ".hpp";

    QFile out1(cpp_name);
    QFile out2(hpp_name);

    if ((out1.open(QFile::WriteOnly | QFile::Truncate)) &&
        (out2.open(QFile::WriteOnly | QFile::Truncate))) {
        QTextStream cpp(&out1);
        QTextStream hpp(&out2);
        QString guard = ui->lineEdit_name->text().toUpper() + "_HPP";
        cpp << "// Generated " << date_time_string().c_str() << "\n\n";
        cpp << "#include \"" << extractFileName(hpp_name) << "\""
            << "\n\n";
        hpp << "// Generated " << date_time_string().c_str() << "\n\n";
        hpp << "#ifndef " << guard << "\n";
        hpp << "#define " << guard << "\n\n";
        hpp << "#include <stdint.h>\n";
        hpp << "#include <string>\n";
        hpp << "\n";

        for (size_t i = 0; i < files.size(); ++i) {
            process(files[i], cpp, hpp);
        }

        hpp << "\n";
        hpp << "#endif // " << guard << "\n";
    }
}

void mainwindow::btn_const_click() {
    QList<QListWidgetItem *> items = ui->listWidget_files->selectedItems();
    foreach (QListWidgetItem *item, items) {
        for (int i = 0; i < ui->listWidget_files->count(); ++i) {
            if (item->text() == ui->listWidget_files->item(i)->text())
                files[i].isConst = !files[i].isConst;
        }
    }
    UpdateListBox();
    isChanged = true;
}

void mainwindow::btn_zip_click() {
    QList<QListWidgetItem *> items = ui->listWidget_files->selectedItems();
    foreach (QListWidgetItem *item, items) {
        for (int i = 0; i < ui->listWidget_files->count(); ++i) {
            if (item->text() == ui->listWidget_files->item(i)->text())
                files[i].isZipped = !files[i].isZipped;
        }
    }
    UpdateListBox();
    isChanged = true;
}

void mainwindow::load_config(const QString fn, FileList &list) {
    try {
        if (fileExists(fn)) {
            current_config = fn;
            tiny::TinyJson json;
            std::string json_str;
            std::ifstream t(fn.toStdString().c_str());
            if (t) {
                std::stringstream ss;
                ss << t.rdbuf(); // reading data
                json_str = ss.str();
            }
            json.ReadJson(json_str);
            tiny::xarray files = json.Get<tiny::xarray>("files");
            for (int i = 0; i < files.Count(); ++i) {
                files.Enter(i);
                bool f = files.Get<int>("const");
                QString val = files.Get<std::string>("name").c_str();
                bool f2 = files.Get<int>("zipped");
                val = to_unix(val);
                val = val.trimmed();
                list.push_back({val, f, f2});
            }
            list = clean_list(list);
            setTitle();
        }
    } catch (...) {
    }
}

void mainwindow::load_config(const QString fn) {
    if (fileExists(fn)) {
        FileList list;
        load_config(fn, list);
        files.clear();
        for (size_t i = 0; i < list.size(); ++i) {
            QString ext = extractFileExt(list[i].fileName);
            if (ext != "files") {
                FileEntry val = list[i];
                val.fileName = val.fileName.trimmed();
                files.push_back(list[i]);
            }
        }
        files = clean_list(files);
        UpdateListBox();
    }
}

void mainwindow::save_config() {
    if (current_config != "") {
        tiny::TinyJson json;
        tiny::TinyJson files_entry;
        for (size_t i = 0; i < files.size(); ++i) {
            tiny::TinyJson entry;
            FileEntry file = files[i];
            file.fileName = file.fileName.trimmed();
            file.fileName = to_json(file.fileName);
            entry[""] =
                "{\"const\":\"" + QString::number((int)file.isConst).toStdString() +
                "\",\"name\":\"" + file.fileName.toStdString() + "\",\"zipped\":\"" +
                QString::number((int)file.isZipped).toStdString() + "\"}";
            files_entry.Push(entry);
        }
        json["files"].Set(files_entry);
        std::string str = json.WriteJson();
        std::ofstream out(current_config.toStdString().c_str());
        out << str << std::endl;
    }
}

void mainwindow::UpdateListBox() {
    if (files.empty()) {
        ui->listWidget_files->clear();
        return;
    }

    timer_skip = true;

    QString path = QFileInfo(files[0].fileName).absolutePath();
    QDir().mkpath(path);

    QString cpp_name = path + "/" + ui->lineEdit_name->text() + ".cpp";

    QDateTime tt;
    if (fileExists(cpp_name))
        tt = getFileDate(cpp_name);
    else
        std::cout << cpp_name.toStdString() << " don't exists" << std::endl;

    QStringList list;
    for (size_t i = 0; i < files.size(); ++i) {
        FileEntry fn = files[i];
        QString fne = fn.fileName;
        if (files[i].isConst)
            fne += " (const)";
        if (files[i].isZipped)
            fne += " (zip)";
        if (fileExists(fn.fileName)) {
            QDateTime ft = getFileDate(files[i].fileName);
            if (ft > tt)
                list.push_back("* " + fne);
            else
                list.push_back("  " + fne);
        } else {
            list.push_back("- " + fne);
        }
    }
    for (size_t i = 0; i < list.size(); ++i) {
        if (i < ui->listWidget_files->count()) {
            if (ui->listWidget_files->item(i)->text() != list[i]) {
                QString s = list[i];
                QListWidgetItem *item = ui->listWidget_files->item(i);
                QColor c(0, 96, 0);
                if (s[0] == '*')
                    c.setRgb(192, 96, 0);
                if (s[0] == '-')
                    c.setRgb(192, 0, 0);
                QBrush b(c);
                item->setForeground(b);
                item->setText(s);
            }
        } else {
            QString s = list[i];
            QListWidgetItem *item = new QListWidgetItem(ui->listWidget_files);
            QColor c(0, 96, 0);
            if (s[0] == '*')
                c.setRgb(191, 96, 0);
            if (s[0] == '-')
                c.setRgb(192, 0, 0);
            QBrush b(c);
            item->setForeground(b);
            item->setText(s);
            ui->listWidget_files->addItem(item);
        }
    }
    for (size_t i = ui->listWidget_files->count() - 1; i >= list.size(); --i)
        ui->listWidget_files->takeItem(i);
    timer_skip = false;
}

void mainwindow::process(const FileEntry &fn, QTextStream &cpp,
                         QTextStream &hpp) {
    if (fileExists(fn.fileName)) {

        QString varname = changeFileExt(extractFileName(fn.fileName), "") + "_" +
                          extractFileExt(fn.fileName);
        if (extractFileExt(fn.fileName) == "ico") {
            std::ifstream in(fn.fileName.toStdString().c_str(),
                             std::ios_base::in | std::ios_base::binary);
            outputBin(varname, in, cpp);
            hpp << "extern const uint8_t " << varname << "[];"
                << "\n";
            hpp << "extern const size_t " << varname << "_len;"
                << "\n";
        } else {
            if (fn.isZipped)
                varname += "_gz";
            std::ifstream in(fn.fileName.toStdString().c_str());
            if (fn.isZipped)
                outputZip(varname, in, cpp);
            else
                output(varname, in, cpp, fn.isConst);
            cpp << "\n";
            if (fn.isZipped) {
                hpp << "extern const uint8_t* const " << varname << ";"
                    << "\n";
                hpp << "extern const size_t " << varname << "_len;"
                    << "\n";
            } else if (fn.isConst)
                hpp << "extern const char* const " << varname << ";"
                    << "\n";
            else
                hpp << "extern std::string " << varname << ";"
                    << "\n";
        }
    }
}

void mainwindow::output(QString vn, std::istream &in, QTextStream &out,
                        const bool &cnst) {
    auto doLine = [&out](std::string &s) {
        QString s1;
        for (size_t i = 0; i < s.size(); ++i) {
            switch (s.at(i)) {
            case '"':
                s1 += "\\\"";
                break;
            case '\\':
                s1 += "\\";
                break;
                // case 9: s1 += "    ";
                // break;
            default:
                s1 += s.at(i);
            }
        }
        s1 = s1.trimmed();
        if (s1.length() > 0)
            out << s1 << "\\n\\"
                << "\n";
        s.clear();
    };

    std::string s;
    std::getline(in, s);
    if (cnst)
        out << "const char* _" << vn << " = "
            << "\"\\"
            << "\n";
    else
        out << "std::string " << vn << " = "
            << "\"\\"
            << "\n";
    while (!in.eof() && !in.fail()) {
        doLine(s);
        std::getline(in, s);
    }
    if (!s.empty())
        doLine(s);
    out << "\";"
        << "\n";
    if (cnst)
        out << "const char* const " << vn << " = _" << vn << ";"
            << "\n";
}

void mainwindow::outputZip(QString vn, std::istream &in, QTextStream &out) {
    std::string s;
    std::getline(in, s);
    out << "const uint8_t _" << vn << "[] = "
        << "{"
        << "\n";
    std::string s2;
    while (!in.eof() && !in.fail()) {
        s2 += s;
        std::getline(in, s);
    }
    if (!s.empty())
        s2 += s;
    std::vector<uint8_t> buf;
    gzipIt(s2.data(), s2.size(), buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        char h[10];
        snprintf(h, sizeof(h), "0x%2.2X, ", (int)buf[i]);
        out << h;
        if ((i + 1) % 16 == 0)
            out << "\\\n";
    }
    out << "};"
        << "\n";
    out << "const uint8_t* const " << vn << " = _" << vn << ";"
        << "\n";
    out << "const size_t " << vn << "_len = " << buf.size() << ";\n";
}

void mainwindow::outputBin(QString vn, std::istream &in, QTextStream &out) {
    char rbuf[128];
    std::vector<uint8_t> buf;
    out << "const uint8_t " << vn << "[] = "
        << "{"
        << "\n";
    in.read(rbuf, sizeof(rbuf));
    while (!in.eof() && !in.fail()) {
        buf.insert(buf.end(), rbuf, rbuf + in.gcount());
        in.read(rbuf, sizeof(rbuf));
    }
    buf.insert(buf.end(), rbuf, rbuf + in.gcount());
    for (size_t i = 0; i < buf.size(); ++i) {
        char h[10];
        snprintf(h, sizeof(h), "0x%2.2X, ", (int)buf[i]);
        out << h;
        if ((i + 1) % 16 == 0)
            out << "\\\n";
    }
    out << "};"
        << "\n";
    out << "const size_t " << vn << "_len = " << buf.size() << ";\n\n";
}

void mainwindow::Timer1Timer() {
    if (!timer_skip)
        UpdateListBox();
    QTimer::singleShot(1000, this, &mainwindow::Timer1Timer);
}
