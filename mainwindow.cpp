#include "mainwindow.h"
#include "tinyjson.hpp"
#include "time_strings.hpp"
#include "file_utils.hpp"

#include <ostream>
#include <istream>
#include <fstream>
#include <QTimer>
#include <QSettings>
#include <QFileDialog>

FileList clean_list(FileList list)
{
    std::sort(list.begin(), list.end());
    FileList::iterator it = std::unique(list.begin(), list.end());
    list.resize(std::distance(list.begin(), it));
    return list;
}

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_mainwindow), timer_skip(false)
{
    ui->setupUi(this);

    connect(ui->pushButton_Load, &QPushButton::released, this, &mainwindow::btn_load_click);
    connect(ui->pushButton_Save, &QPushButton::released, this, &mainwindow::btn_save_click);
    connect(ui->pushButton_Add, &QPushButton::released, this, &mainwindow::btn_add_click);
    connect(ui->pushButton_Del, &QPushButton::released, this, &mainwindow::btn_del_click);
    connect(ui->pushButton_Clear, &QPushButton::released, this, &mainwindow::btn_clear_click);
    connect(ui->pushButton_Gen, &QPushButton::released, this, &mainwindow::btn_gen_click);

    if (QApplication::arguments().count() > 1)
    {
        QString config = QApplication::arguments().at(1);
        load_config(config);
    }
    else
    {
        QSettings mySettings("StestrupOld", "mainwindow");
        QString config = mySettings.value("config").toString();
        if (!config.isEmpty())
            load_config(config);
    }

    QTimer::singleShot(1000, this, &mainwindow::Timer1Timer);
}

mainwindow::~mainwindow()
{
    QSettings mySetting;
    QSettings mySettings("StestrupOld", "mainwindow");
    mySettings.setValue("config", current_config);
}

void mainwindow::file_exit()
{
    QApplication::quit();
}

void mainwindow::setTitle()
{
    QString title = "mainwindow  ";
    QString file = current_config;
    if (file.size() > 64)
        file = file.mid(0, 16) + "..." + file.mid(file.size() - 40);
    title += file;
    setWindowTitle(title);
    ui->listWidget_files->setToolTip(current_config);
    ui->listWidget_files->setToolTipDuration(2000);
}

void mainwindow::btn_load_click()
{
    std::unique_ptr<QFileDialog> form{std::make_unique<QFileDialog>()};
    const QStringList filters({"File list (*.files)", "All files(*.*)"});
    form->setDirectory(QFileInfo(current_config).dir());
    if (form->exec())
    {
        files.clear();
        FileList list;
        QStringList l = form->selectedFiles();
        for (size_t i = 0; i < l.count(); ++i)
            list.push_back(l[i]);
        clean_list(list);
        for (size_t i = 0; i < list.size(); ++i)
        {
            QString ext = extractFileExt(list[i]);
            if (ext == "files")
                load_config(list[i], list);
        }
        for (int i = 0; i < list.size(); ++i)
        {
            QString ext = extractFileExt(list[i]);
            if (ext != "files")
                files.push_back(list[i]);
        }
        clean_list(files);
        UpdateListBox();
    }
}

void mainwindow::btn_save_click()
{
    const QStringList filters({"File list (*.files)", "All files(*.*)"});
    QString dir;
    QString file;

    if (current_config != "")
    {
        dir = extractFileDir(current_config);
        file = extractFileName(current_config);
    }
    else
    {
        if (files.size() > 0)
        {
            dir = extractFileDir(files[0]);
            file = "maketemplates.files";
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), dir, tr("File list (*.files)"));
    current_config = fileName;
    setTitle();
    save_config();
}

void mainwindow::btn_add_click()
{
    std::unique_ptr<QFileDialog> form{std::make_unique<QFileDialog>()};
    form->setNameFilter(tr("Source files (*.html *.htm *.js *.css *.json *.svg *.files);;File list (*.files);;All files (* .*)"));
    form->setFileMode(QFileDialog::ExistingFiles);
    form->setDirectory(QFileInfo(current_config).dir());
    if (form->exec())
    {
        FileList list = files;
        QStringList l = form->selectedFiles();
        for (size_t i = 0; i < l.count(); ++i)
            list.push_back(l[i]);
        for (size_t i = 0; i < list.size(); ++i)
        {
            QString ext = extractFileExt(list[i]);
            if (ext == "files")
                load_config(list[i], list);
        }
        files.clear();
        for (int i = 0; i < list.size(); ++i)
        {
            QString ext = extractFileExt(list[i]);
            if (ext != "files")
                files.push_back(list[i]);
        }
        files = clean_list(files);
    }
    UpdateListBox();
}

void mainwindow::btn_del_click()
{
    QList<QListWidgetItem *> items = ui->listWidget_files->selectedItems();
    foreach (QListWidgetItem *item, items)
    {
        delete ui->listWidget_files->takeItem(ui->listWidget_files->row(item));
    }
    files.clear();
    for (int i = 0; i < ui->listWidget_files->count(); ++i)
    {
        files.push_back(ui->listWidget_files->item(i)->text());
    }
    UpdateListBox();
}

void mainwindow::btn_clear_click()
{
    ui->listWidget_files->clear();
    files.clear();
    current_config.clear();
    setTitle();
    UpdateListBox();
}

void mainwindow::btn_gen_click()
{
    if (files.size() == 0)
        return;

    QString path = QFileInfo(files[0]).absolutePath();
    QDir().mkpath(path);

    QString cpp_name = path + "/" + ui->lineEdit_name->text() + ".cpp";
    QString hpp_name = path + "/" + ui->lineEdit_name->text() + ".hpp";

    QFile out1(cpp_name);
    QFile out2(hpp_name);

    if ((out1.open(QFile::WriteOnly | QFile::Truncate)) && (out2.open(QFile::WriteOnly | QFile::Truncate)))
    {
        QTextStream cpp(&out1);
        QTextStream hpp(&out2);
        QString guard = ui->lineEdit_name->text().toUpper() + "_HPP";
        cpp << "// Generated " << date_time_string().c_str() << "\n\n";
        cpp << "#include \"" << extractFileName(hpp_name) << "\""
            << "\n\n";
        hpp << "// Generated " << date_time_string().c_str() << "\n\n";
        hpp << "#ifndef " << guard << "\n";
        hpp << "#define " << guard << "\n\n";
        hpp << "#include <string>"
            << "\n\n";

        for (size_t i = 0; i < files.size(); ++i)
        {
            process(files[i], cpp, hpp);
        }

        hpp << "\n";
        hpp << "#endif // " << guard << "\n";
    }
}

void mainwindow::load_config(const QString fn, FileList &list)
{
    if (fileExists(fn))
    {
        current_config = fn;
        tiny::TinyJson json;
        std::string json_str;
        std::ifstream t(fn.toStdString().c_str());
        if (t)
        {
            std::stringstream ss;
            ss << t.rdbuf(); // reading data
            json_str = ss.str();
        }
        json.ReadJson(json_str);
        tiny::xarray files = json.Get<tiny::xarray>("files");
        for (int i = 0; i < files.Count(); ++i)
        {
            files.Enter(i);
            QString val = files.Get<std::string>().c_str();
            val = to_unix(val);
            list.push_back(val);
        }
        list = clean_list(list);
        setTitle();
    }
}

void mainwindow::load_config(const QString fn)
{
    if (fileExists(fn))
    {
        FileList list;
        load_config(fn, list);
        files.clear();
        for (size_t i = 0; i < list.size(); ++i)
        {
            QString ext = extractFileExt(list[i]);
            if (ext != "files")
                files.push_back(list[i]);
        }
        files = clean_list(files);
        UpdateListBox();
    }
}

void mainwindow::save_config()
{
    if (current_config != "")
    {
        tiny::TinyJson json;
        tiny::TinyJson files_entry;
        for (size_t i = 0; i < files.size(); ++i)
        {
            tiny::TinyJson entry;
            QString file = files[i];
            file = to_json(file);
            entry[""] = "\"" + file.toStdString() + "\"";
            files_entry.Push(entry);
        }
        json["files"].Set(files_entry);
        std::string str = json.WriteJson();
        std::ofstream out(current_config.toStdString().c_str());
        out << str << std::endl;
    }
}

void mainwindow::UpdateListBox()
{
    if (files.empty())
        return;

    timer_skip = true;

    QString path = QFileInfo(files[0]).absolutePath();
    QDir().mkpath(path);

    QString cpp_name = path + "/" + ui->lineEdit_name->text() + ".cpp";

    QDateTime tt;
    if (fileExists(cpp_name))
        tt = getFileDate(cpp_name);
    else
        std::cout << cpp_name.toStdString() << " don't exists" << std::endl;

    FileList list;
    for (size_t i = 0; i < files.size(); ++i)
    {
        QString fn = files[i];
        if (fileExists(fn))
        {
            QDateTime ft = getFileDate(files[i]);
            if (ft > tt)
                list.push_back("* " + fn);
            else
                list.push_back("  " + fn);
        }
        else
        {
            list.push_back("- " + fn);
        }
    }
    for (size_t i = 0; i < list.size(); ++i)
    {
        if (i < ui->listWidget_files->count())
        {
            if (ui->listWidget_files->item(i)->text() != list[i])
            {
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
        }
        else
        {
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
    timer_skip = false;
}

void mainwindow::process(const QString fn, QTextStream &cpp, QTextStream &hpp)
{
    if (fileExists(fn))
    {
        QString varname = changeFileExt(extractFileName(fn), "") + "_" +
                          extractFileExt(fn);
        std::ifstream in(fn.toStdString().c_str());
        output(varname, in, cpp);
        cpp << "\n";
        hpp << "extern std::string " << varname << ";"
            << "\n";
    }
}

void mainwindow::output(QString vn, std::istream &in, QTextStream &out) {
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
    out << "std::string " << vn << " = "
        << "\"\\"
        << "\n";
    while (!in.eof() && !in.fail()) {
        doLine(s);
        // QString s1;
        // for (size_t i = 0; i < s.size(); ++i)
        // {
        //     switch (s.at(i))
        //     {
        //     case '"':
        //         s1 += "\\\"";
        //         break;
        //     case '\\':
        //         s1 += "\\";
        //         break;
        //         // case 9: s1 += "    ";
        //         // break;
        //     default:
        //         s1 += s.at(i);
        //     }
        // }
        // s1 = s1.trimmed();
        // if (s1.length() > 0)
        //     out << s1 << "\\n\\"
        //         << "\n";
        // s.clear();
        std::getline(in, s);
    }
    if (!s.empty())
        doLine(s);
    out << "\";"
        << "\n";
}

void mainwindow::Timer1Timer()
{
    if (!timer_skip)
        UpdateListBox();
    QTimer::singleShot(1000, this, &mainwindow::Timer1Timer);
}
