#include "printdatabase.h"
#include "base.h"
#include <QApplication>
#include <algorithm>
#include <cmath>
#include <format>
#include <vector>

PrintDatabase::PrintDatabase() {
  _db = QSqlDatabase::addDatabase("QSQLITE");
  _db.setDatabaseName(QCoreApplication::applicationDirPath() +
                      "/PrinterInfo.db");

  if (_db.open()) {
    _query = std::make_unique<QSqlQuery>(_db);

  } else {
    QApplication::quit();
  }
}

std::tuple<bool, std::string>
PrintDatabase::printerConfigInsert(const PrinterConfig &pc) {

  try {
    QString queryExit =
        QString(R"(SELECT 1 FROM printer_config where Name ='%1';)")
            .arg(QString::fromStdString(pc.Name));
    if (_query->exec(queryExit)) {
      if (_query->next()) {
        return std::make_tuple(false, "不能有重复的名称！");
      }
    }
    auto insert_sql = std::format(
        R"(
                  INSERT INTO printer_config (
                                  Name,
                                  PrinterName,
                                  TopMargin,
                                  BottomMargin,
                                  LeftMargin,
                                  RightMargin,
                                  Orientation,
                                  PaperName
                              )
                              VALUES (
                              '{}',
                              '{}',
                               {},
                               {},
                               {},
                               {},
                              '{}',
                              '{}'
                              );
            )",
        pc.Name, pc.PrinterName, pc.TopMargin, pc.BottomMargin, pc.LeftMargin,
        pc.RightMargin, pc.Orientation, pc.PaperName);
    if (_query->exec(QString::fromStdString(insert_sql))) {
      return std::make_tuple(true, "");
    }
    throw;

  } catch (...) {
    return std::make_tuple(false, "未知的错误发生了~");
  }
}

std::tuple<bool, std::string> PrintDatabase::printerConfigDel(const int Id) {

  try {
    QString delSql =
        QString(R"(delete from printer_config where Id = %1;)").arg(Id);

    if (_query->exec(delSql)) {
      return std::make_tuple(true, "");
    }

    return std::make_tuple(false, "删除失败！");

  } catch (...) {
    return std::make_tuple(false, "未知的错误发生了~");
  }
}

std::tuple<bool, std::string>
PrintDatabase::printerConfigUpdate(const PrinterConfig &pc) {
  try {
    auto update_sql = std::format(R"(
                update printer_config
                       set Name = '{}',
                       PrinterName = '{}',
                       PaperName = '{}',
                       Orientation = '{}',
                       TopMargin = {},
                       BottomMargin = {},
                       LeftMargin = {},
                       RightMargin = {}
                       where Id = {})",
                                  pc.Name, pc.PrinterName, pc.PaperName,
                                  pc.Orientation, pc.TopMargin, pc.BottomMargin,
                                  pc.LeftMargin, pc.RightMargin, pc.Id);
    if (_query->exec(QString::fromStdString(update_sql))) {
      return std::make_tuple(true, "");
    } else {
      return std::make_tuple(false, _query->lastError().text().toStdString());
    }

  } catch (...) {
    return std::make_tuple(false, "未知的错误发生了~");
  }
}

const std::vector<PrinterConfig> &&
PrintDatabase::printerConfigQueryById(int Id) {
  std::vector<PrinterConfig> pcs;
  try {
    QString querySql;
    if (Id == -1) { // 查询全部
      querySql = QString(R"(SELECT * FROM printer_config)");
      if (_query->exec(querySql)) {
        while (_query->next()) {
          pcs.push_back(PrinterConfig(_query.get()));
        }
      }
    } else {
      querySql = QString(R"(SELECT * FROM printer_config where Id=%1)").arg(Id);
      if (_query->exec(querySql)) {
        while (_query->next()) {
          pcs.push_back(PrinterConfig(_query.get()));
        }
      }
    }
    return std::move(pcs);
  } catch (...) {
    return std::move(pcs);
  }
}

const std::vector<PrinterConfig> &&
PrintDatabase::printerConfigQueryByName(const QString &Name) {
  std::vector<PrinterConfig> pcs;
  try {
    QString querySql =
        QString(R"(SELECT * FROM printer_config where Name='%1')").arg(Name);
    if (_query->exec(querySql)) {
      while (_query->next()) {
        pcs.push_back(PrinterConfig(_query.get()));
      }
    }
    return std::move(pcs);

  } catch (...) {
    return std::move(pcs);
  }
}

const std::vector<PrintedPage> &&
PrintDatabase::printedPageQuery(int page_size_, int page_index_) {
  std::vector<PrintedPage> pps;
  try {
    QString query_sql = QString(R"(
   select  * from printed_page order by Id limit %1 offset %2
  )")
                            .arg(page_size_)
                            .arg(page_size_ * page_index_);
    if (_query->exec(query_sql)) {
      while (_query->next()) {
        pps.push_back(PrintedPage(_query.get()));
      }
    }
    return std::move(pps);
  } catch (...) {
    return std::move(pps);
  }
}

bool PrintDatabase::printedPageInsert(const PrintedPage &pp_) {
  try {

    auto query_sql =
        std::format(R"(
          insert into printed_page 
          (PrintTime,FromIp,FromType,PageName,ConfigId,PrintMode,IsSuccess)
          values('{}','{}','{}','{}',{},'{}',{})
  )",
                    pp_.PrintTime, pp_.FromIp, pp_.FromType, pp_.PageName,
                    pp_.ConfigId, pp_.PrintMode, pp_.IsSuccess);

    if (_query->exec(QString::fromStdString(query_sql))) {
      return true;
    }
    return false;
  } catch (...) {
    return false;
  }
}
