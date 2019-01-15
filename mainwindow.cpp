#include "mainwindow.h"
#include "previewpage.h"
#include <QtWidgets>
#include <QWebChannel>

MainWindow::MainWindow()
    : textEditor(new QPlainTextEdit), contentPreview(new QWebEngineView)
{
    setWindowIcon(QIcon(":/images/markdowneditor.png"));
//    textEditor->setFont(QFont("Microsoft YaHei",10));
    textEditor->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    textEditor->setStyleSheet("selection-background-color: DodgerBlue;"
                            "selection-color: white");

    createActions();
    createStatusBar();
    readSettings();            //读取程序上一次关闭时的窗口状态
    setCurrentFile(QString()); //初始化文件名

    QHBoxLayout *windowLayout = new QHBoxLayout;
    windowLayout->addWidget(textEditor);
    windowLayout->addWidget(contentPreview);
    windowLayout->setStretchFactor(textEditor, 1);
    windowLayout->setStretchFactor(contentPreview, 1);
    QWidget *centralWindow = new QWidget;
    centralWindow->setLayout(windowLayout);
    setCentralWidget(centralWindow);

    PreviewPage *page = new PreviewPage(this);
    contentPreview->setPage(page);

    connect(textEditor, &QPlainTextEdit::textChanged,
            [this]() { curContent.setText(textEditor->toPlainText()); });

    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject(QStringLiteral("content"), &curContent); //用当前文档内容注册一个webchannel
    page->setWebChannel(channel);

    contentPreview->setUrl(QUrl("qrc:/index.html"));

    QFile defaultTextFile(":/default.md");
    defaultTextFile.open(QIODevice::ReadOnly);
    textEditor->setPlainText(defaultTextFile.readAll());

    //查找窗口初始化
    findDlg=new QDialog(this);
    findDlg->setWindowTitle(tr("Find"));
    findLineEdit=new QLineEdit(findDlg);
    findLineEdit->setFont(QFont("微软雅黑",10));
    QPushButton *nextBtn= new QPushButton(tr("Find Next"), findDlg);
    QVBoxLayout *findLayout= new QVBoxLayout(findDlg);
    findLayout->addWidget(findLineEdit);
    findLayout->addWidget(nextBtn);
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::showFindText);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(maybeSave()) {
        writeSettings(); //在程序关闭前保存此时的窗口状态
        event->accept();
    }
    else {
        event->ignore();
    }
}

/*********************************此处开始定义所有的槽的操作************************************************/

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEditor->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::open()
{
    if(maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                           tr("Open File"), "", tr("Markdown file (*.md)"));
        if(!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

bool MainWindow::save()
{
    if(isUntitled) {
        return saveAs();
    }
    else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save File"), curFile, tr("Markdown file (*.md)"));
    if(!fileName.isEmpty())
    {
        return saveFile(fileName);
    }
    return false;
}

void MainWindow::close()
{
    if(maybeSave())
    {
        textEditor->setVisible(false);
        qApp->quit();
    }
}

void MainWindow::about()
{
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("Markdown Editor")+tr(" v1.0"));
    aboutBox.setIcon(QMessageBox::Information);
    aboutBox.setText(tr("A simple markdown editor,writen by Xiri."));
    aboutBox.exec();
}

void MainWindow::find()
{
    findDlg->show();
}

void MainWindow::showFindText() //从textEdit里查找输入的字符串
{
    QString findStr=findLineEdit->text();
    if (!textEditor->find(findStr))
    {
        QMessageBox::warning(this, tr("Find"),
                 tr("Can not find text \"%1\"").arg(findStr));
    }
}

/*********************************此处开始定义所有实际的文件操作*********************************************/

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Markdown Editor"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");   //以UTF-8格式打开文件
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEditor->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Markdown Editor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");  //保存为UTF-8格式
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEditor->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}


bool MainWindow::maybeSave() //判断当前文件是否需要保存
{
    if(textEditor->document()->isModified())
    {
        QMessageBox box;
        box.setWindowTitle(tr("Markdown Editor"));
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("The document has been modified.\n"
                       "Do you want to save your changes?"));
        QPushButton *yesBtn = box.addButton(tr("Save"),
                                            QMessageBox::YesRole);
        box.addButton(tr("Discard"),QMessageBox::NoRole);
        QPushButton *cancelBtn = box.addButton(tr("Cancel"),
                                               QMessageBox::RejectRole);
        box.exec();
        if(box.clickedButton() == yesBtn)
            return save();
        else if(box.clickedButton() == cancelBtn)
            return false;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile=fileName;
    textEditor->document()->setModified(false);
    isUntitled=false; //默认为存在对应文件

    if (curFile.isEmpty()) //fileName为空时设置默认文件名
    {
        curFile = tr("untitled");
        isUntitled=true; //此时没有与编辑器内内容对应的文件
    }
    setWindowFilePath(curFile);
}

/*********************************此处开始定义所有界面操作**************************************************/

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File(&F)"));
//    QToolBar *fileToolBar = addToolBar(tr("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new",QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("New(&N)"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
//    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("Open(&O)"), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
//    fileToolBar->addAction(openAct);

    fileMenu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("Save(&S)"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
//    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as", QIcon(":/images/saveAs.png"));
    QAction *saveAsAct = new QAction(saveAsIcon, tr("Save As(&A)"), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit", QIcon(":/images/close.png"));
    QAction *exitAct = new QAction(exitIcon, tr("Exit(&Q)"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &MainWindow::close);
    fileMenu->addAction(exitAct);


    QMenu *editMenu = menuBar()->addMenu(tr("Edit(&E)"));
//    QToolBar *editToolBar = addToolBar(tr("Edit"));

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
    QAction *cutAct = new QAction(cutIcon, tr("Cut(&X)"),this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(cutAct, &QAction::triggered, textEditor, &QPlainTextEdit::cut);
    editMenu->addAction(cutAct);
//    editToolBar->addAction(cutAct);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    QAction *copyAct = new QAction(copyIcon,tr("Copy(&C)"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copyAct, &QAction::triggered, textEditor, &QPlainTextEdit::copy);
    editMenu->addAction(copyAct);
//    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    QAction *pasteAct = new QAction(pasteIcon, tr("Paste(&V)"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(pasteAct, &QAction::triggered, textEditor, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAct);
//    editToolBar->addAction(pasteAct);

    editMenu->addSeparator();

    const QIcon findIcon = QIcon::fromTheme("edit-find", QIcon(":/images/find.png"));
    QAction *findAct = new QAction(findIcon, tr("Find(&F)"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setStatusTip(tr("Find text from the the document"));
    connect(findAct, &QAction::triggered, this, &MainWindow::find);
    editMenu->addAction(findAct);
//    editToolBar->addAction(findAct);


    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEditor, &QPlainTextEdit::copyAvailable, cutAct, &QAction::setEnabled);
    connect(textEditor, &QPlainTextEdit::copyAvailable, copyAct, &QAction::setEnabled);

    QMenu *helpMenu = menuBar()->addMenu(tr("Help(&H)"));

    const QIcon aboutIcon = QIcon::fromTheme("help-about", QIcon(":/images/about.png"));
    QAction *aboutAct = new QAction(aboutIcon,tr("About"), this);
    aboutAct->setStatusTip(tr("About this editor"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::writeSettings()
{
//    QSettings settings(QCoreApplication::applicationName());
//    settings.setValue("geometry", saveGeometry());
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize((availableGeometry.width() * 1190) / 1920, (availableGeometry.height() * 842) / 1080);
        move((availableGeometry.width() - width()) /2,
             (availableGeometry.height() - height()) / 2);
    }
    else
    {
        restoreGeometry(geometry);
    }
}
