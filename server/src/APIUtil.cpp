#include "APIUtil.h"
#include "TradeRecord.h"
#include "Logger.h"

#include <sstream>

const std::string APIUtil::TAG("APIUtil");

bool APIUtil::securityTryLock(const std::string &symbol, bool &result) {
    std::ostringstream s;
    auto stmt = getStmt();
    ResultSetPtr results;

    s << "CALL security_try_lock('" << symbol << "', @result)";

    if (!execute(stmt, s.str()) || !executeQuery(stmt, "SELECT @result AS _result", results)) {
        return false;
    }
    results->next();
    result = results->getBoolean("_result");
    return true;
}

bool APIUtil::securityUnlock(const std::string &symbol) {
    std::ostringstream s;
    s << "CALL security_unlock('" << symbol << "')";
    return execute(s.str());
}

bool APIUtil::securityUpdatePrice(const SecurityStatus &security) {
    std::ostringstream s;
    s << "CALL security_update_price('"
    << security.symbol << "',"
    << security.price << ")";
    return execute(s.str());
}

bool APIUtil::securityQuery(const std::string &symbol, SecurityStatus &result) {
    std::ostringstream s;
    ResultSetPtr results;
    auto stmt = getStmt();
    s << "CALL security_query('" << symbol << "')";
    if (!executeQuery(stmt, s.str(), results)) {
        return false;
    }
    while (results->next())
        result = SecurityStatus(results);
    stmt->getMoreResults();
    return true;
}

bool APIUtil::securityUpdateTradingStatus(const std::string &symbol, const kSecurityTradingStatus status) {
    std::ostringstream s;
    s << "CALL security_update_trading_status('"
    << symbol << "'," << static_cast<int>(status) << ")";
    return execute(s.str());
}

bool APIUtil::orderbookPut(const Quote &quote) {
    std::ostringstream s;
    s << "CALL orderbook_put("
    << "'" << quote.symbol.c_str() << "',"
    << "'" << quote.client.c_str() << "',"
    << "'" << quote.client_order_id.c_str() << "',"
    << "'" << static_cast<char>(quote.side) << "',"
    << quote.price << ","
    << quote.quantity << ")";
    return execute(s.str());
}

bool APIUtil::orderbookDelete(const int quote_id) {
    std::ostringstream s;
    s << "CALL orderbook_delete(" << quote_id << ")";
    return execute(s.str());
}

bool APIUtil::orderBookQuery(const std::string &symbol, kTradeSide side, bool query_highest, Quote &result) {
    std::ostringstream s;
    auto stmt = getStmt();
    ResultSetPtr results;
    s << "CALL orderbook_query_"
    << (query_highest ? "highest" : "lowest")
    << "('" << symbol << "','"
    << static_cast<char>(side) << "')";
    if (!executeQuery(stmt, s.str(), results)) {
        return false;
    }
    while (results->next())
        result = Quote(results);
    stmt->getMoreResults();
    return true;
}

bool APIUtil::orderbookQueryMatch(const std::string &symbol, ResultSetPtr &results) {
    std::ostringstream s;
    s << "CALL orderbook_query_match('" << symbol << "')";
    return executeQuery(s.str(), results);
}

bool APIUtil::orderbookUpdate(const int quote_id, const int quantity) {
    std::ostringstream s;
    s << "CALL orderbook_update(" << quote_id << "," << quantity << ")";
    return execute(s.str());
}

bool APIUtil::systemStatusIsRunning(bool &result) {
    auto stmt = getStmt();
    ResultSetPtr results;

    if (!execute(stmt, "CALL systemstatus_is_running(@result)") ||
        !executeQuery(stmt, "SELECT @result AS _result", results)) {
        return false;
    }
    while (results->next())
        result = results->getBoolean("_result");
    return true;
}

bool APIUtil::tradeRecordPut(const TradeRecord &record) {
    std::ostringstream s;
    s << "CALL traderecord_put("
    << record.order_buy << ","
    << record.order_sell << ","
    << record.price << ","
    << record.quantity << ")";
    return execute(s.str());
}


/*
bool APIUtil::securityStartTrading(const std::string &symbol) {
    std::ostringstream s;
    s << "CALL security_start_trading('" << symbol << "')";
    return execute(s.str());
}

bool APIUtil::securityStopTrading(const std::string &symbol) {
    std::ostringstream s;
    s << "CALL security_stop_trading('" << symbol << "')";
    return execute(s.str());
}*/

bool APIUtil::checkConnection() {
    if (conn_->isClosed()) {
        try {
            conn_->reconnect();
        } catch (const sql::SQLException &e) {
            logError(e, "reconnect");
            return false;
        }
    }
    return true;
}

bool APIUtil::execute(const std::string &s) {
    return execute(getStmt(), s);
}

bool APIUtil::execute(StmtPtr stmt, const std::string &s) {
    try {
        stmt->execute(s);
    } catch (const sql::SQLException &e) {
        logError(e, s);
        return false;
    }
    return true;
}

APIUtil::StmtPtr APIUtil::getStmt() {
    checkConnection();
    return std::shared_ptr<sql::Statement>(conn_->createStatement());
}

bool APIUtil::executeQuery(const std::string &s, std::shared_ptr<sql::ResultSet> &results) {
    auto stmt = getStmt();
    bool success = executeQuery(getStmt(), s, results);
    stmt->getMoreResults();//free last results;
    return success;
}

bool APIUtil::executeQuery(APIUtil::StmtPtr stmt, const std::string &s, std::shared_ptr<sql::ResultSet> &results) {
    try {
        results.reset(stmt->executeQuery(s));
    } catch (const sql::SQLException &e) {
        logError(e, s);
        return false;
    }
    return true;
}

void APIUtil::logError(const sql::SQLException &e, const std::string &query_string) {
    std::ostringstream os;
    os << "APIUtil: SQLException when executing '" << query_string
    << "'. # ERR: " << e.what()
    << " (MySQL error code: " << e.getErrorCode() <<
    ", SQLState: " << e.getSQLState() << " )";
    Logger::getLogger()->error(TAG, os.str());
}