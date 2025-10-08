#ifndef mainwindow_HPP
#define mainwindow_HPP

#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <vector>
#include <string>

typedef std::vector<QString> FileList;

class mainwindow : public QMainWindow
{
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

private:
    std::unique_ptr<Ui_mainwindow> ui;
    QMenu *fileMenu;
    std::unique_ptr<QAction> exitAct;

    QString current_config;
    FileList files;

    bool timer_skip;

    void setTitle();

    void load_config(const QString fn, FileList &list);
    void load_config(const QString fn);
    void save_config();

    void process(const QString fn, QTextStream &cpp, QTextStream &hpp);
    void output(QString vn, std::istream &in, QTextStream &out);

    void Timer1Timer();

    void UpdateListBox();
};

#endif // mainwindow_HPP
