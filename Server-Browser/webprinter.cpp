#include "webprinter.h"
#include <QDebug>

WebPrinter::WebPrinter()
    : _avaliable_printer_info(QPrinterInfo::availablePrinters()) {

  //  qDebug() << _avaliable_printer_info.at(0).printerName();

  //  qDebug() << _avaliable_printer_info.at(0).supportedPageSizes();

  //    for(auto& x: _avaliable_printer_info) {
  //        qDebug()<<x.printerName();
  //        qDebug()<<x.supportedPageSizes();
  //        qDebug()<<"#######################";

  //    }
  connect(&_render_view, &QWebEngineView::loadFinished, this,
          &WebPrinter::SlotLoadFinishTorint);
  connect(&_render_view, &QWebEngineView::printRequested, this,
          &WebPrinter::SlotJsPrintRequestToPrint);
  connect(&_render_view, &QWebEngineView::printFinished, this,
          &WebPrinter::SlotPrintFinshed);

  connect(&_timeout_listen, &QTimer::timeout, this,
          &WebPrinter::SlotPrintRequestTimeOut);

  connect(this, &WebPrinter::signalGuiThreadToWork, this,
          &WebPrinter::SlotMoveToGUIThreadWork, Qt::QueuedConnection);
  //_timeout_listen.start(200000);
}

void WebPrinter::AddPrintWebPageToQueue(
    const QUrl &url, const QString &printerInfoName, PrintModel model,
    const QString &PageName, const QMarginsF &marginsF,
    QPageLayout::Orientation oriention,
    std::function<void(bool, const QString &)> callback) {
  _webpagelist.push(std::make_tuple(url, printerInfoName, model, PageName,
                                    marginsF, oriention, callback));
}

const QList<QPrinterInfo> &WebPrinter::GetAvaliablePrinterInfo() {
  return _avaliable_printer_info;
}

void WebPrinter::UpdatePrinterInfo() {
  _avaliable_printer_info = QPrinterInfo::availablePrinters();
}

void WebPrinter::ToPrint() {
  if (_webpagelist.empty()) {

    _currentState = PrintState::IsWaiting;
    return;
  }

  auto current = _webpagelist.front();
  _webpagelist.pop();

  for (auto &x : _avaliable_printer_info) {
    if (x.printerName() == std::get<1>(current)) {

      for (auto &y : x.supportedPageSizes()) {
        if (y.name() == std::get<3>(current)) {
          QPageLayout pagelayout;
          pagelayout.setPageSize(y);
          pagelayout.setMargins(std::get<4>(current));
          pagelayout.setOrientation(std::get<5>(current));
          QPrinter *currentPrinter = nullptr;
          bool ishasIn = false;
          for (auto &_p : _printers) {
            if (_p->printerName() == x.printerName()) {
              ishasIn = true;
              currentPrinter = _p;
              break;
            }
          }
          if (!ishasIn) {
            currentPrinter = new QPrinter(x, QPrinter::HighResolution);
            _printers.push_back(currentPrinter);
          }

          _currentpage =
              std::make_tuple(std::get<0>(current), currentPrinter,
                              std::get<2>(current), std::get<6>(current));
          _timeout_listen.start(200000);
          _render_view.load(std::get<0>(_currentpage));
          return;
        }
      }
    }
  }
  return std::get<6>(current)(false, QString("?????????????????????????????????????????????"));
}

void WebPrinter::StartWork() {
  if (_currentState == PrintState::IsWorking) {
    return;
  }
  _currentState = PrintState::IsWorking;
  ToPrint();
}

void WebPrinter::SlotLoadFinishTorint(bool isSuccess) {
  if (isSuccess) {
    if (std::get<2>(_currentpage) == PrintModel::LoadAchieve) {
      return _render_view.print(std::get<1>(_currentpage));
    }

  } else {
    std::get<3>(_currentpage)(false, QString("?????????????????????"));
  }
}

void WebPrinter::SlotJsPrintRequestToPrint() {
  if (std::get<2>(_currentpage) == PrintModel::JsPrintRequest) {
    _render_view.print(std::get<1>(_currentpage));
  }
}

void WebPrinter::SlotPrintRequestTimeOut() {
  ///
  ///
  ///??????????????????????????????????????????
  std::get<3>(_currentpage)(false, QString("????????????????????????"));
  ToPrint();
}

void WebPrinter::SlotPrintFinshed(bool isSuccess) {
  if (isSuccess) {
    std::get<3>(_currentpage)(true, QString());
  } else {
    std::get<3>(_currentpage)(false, QString("???????????????????????????????????????"));
  }
  _timeout_listen.stop();
  ToPrint();
}

void WebPrinter::SlotMoveToGUIThreadWork() { StartWork(); }
