#ifndef EXCHANGESIMULATOR_SERVER_QUOTEREQUESTHANDLER_H
#define EXCHANGESIMULATOR_SERVER_QUOTEREQUESTHANDLER_H


#include "Fix42.h"
#include "MessageHandler.h"
#include "APIUtil.h"

#include <vector>
#include <memory>

class QuoteRequestHandler : public MessageHandler {
public:
    QuoteRequestHandler(APIUtil::ConnPtr conn) : MessageHandler(conn) { }

    virtual std::vector<Fix42::MessagePtr> accept(const Fix42::MessagePtr message) override;

private:
    void querySingle(const std::string &symbol, std::vector<Fix42::MessagePtr> &results);

    void addToQuoteList(std::shared_ptr<Fix42::RepeatGroup> &quote_list, const Quote &quote) const;

};


#endif //EXCHANGESIMULATOR_SERVER_QUOTEREQUESTHANDLER_H
