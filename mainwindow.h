#ifndef mainwindow_HPP
#define mainwindow_HPP

#include "ui_mainwindow.h"

#include <QDir>
#include <QFileSystemModel>
#include <QMainWindow>
#include <vector>

struct FileEntry {
    bool isConst;
    QString fileName;
    bool isZipped;
    FileEntry(const QString &name = "", const bool cnst = false,
              const bool zip = false):fileName(name), isConst(cnst), isZipped(zip) {}
    bool operator==(const FileEntry &fe) {
        return (fileName == fe.fileName) && (isConst == fe.isConst) &&
               (isZipped == fe.isZipped);
    }
    bool operator<(const FileEntry &fe) { return (fileName < fe.fileName); }
};

typedef std::pair<bool, QString> FileEntry1;
typedef std::vector<FileEntry> FileList;

class mainwindow : public QMainWindow {
    Q_OBJECT

public:
    mainwindow(QWidget *parent = nullptr);
    virtual ~mainwindow();

private slots:
    void file_exit();
    void btn_load_click();
    void btn_save_click();
    void btn_add_click();
    void btn_del_click();
    void btn_clear_click();
    void btn_gen_click();
    void btn_const_click();
    void btn_zip_click();
    void closeEvent (QCloseEvent *event);

private:
    std::unique_ptr<Ui_mainwindow> ui;
    QMenu *fileMenu;
    std::unique_ptr<QAction> exitAct;

    QString current_config;
    FileList files;

    bool timer_skip;

    bool isChanged;

    void setTitle();

    void load_config(const QString fn, FileList &list);
    void load_config(const QString fn);
    void save_config();

    void process(const FileEntry &fn, QTextStream &cpp, QTextStream &hpp);
    void output(QString vn, std::istream &in, QTextStream &out, const bool &cnst);
    void outputZip(QString vn, std::istream &in, QTextStream &out);
    void outputBin(QString vn, std::istream &in, QTextStream &out);

    void Timer1Timer();

    void UpdateListBox();
};

#endif // mainwindow_HPP
