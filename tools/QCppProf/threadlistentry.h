#ifndef THREADLISTENTRY_H
#define THREADLISTENTRY_H

#include <QWidget>

namespace Ui {
class ThreadListEntry;
}

class ThreadListEntry : public QWidget
{
	Q_OBJECT
	
public:
	explicit ThreadListEntry(int thread_id, QString text, QColor color, QWidget *parent = 0);
	~ThreadListEntry();

	bool isThreadEnabled();
	QColor getColor() const { return t_color;}
	
private slots:
	void on_colorSelection_released();

	void on_threadState_stateChanged(int arg1);

signals:
	void changed();

private:
	Ui::ThreadListEntry *ui;
	int id;
	QString t_text;
	QColor t_color;
};

#endif // THREADLISTENTRY_H
