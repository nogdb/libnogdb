/**
 *
 * An example of a program using libnogdb for graph manipulation and traversal
 * By pj4dev - https://github.com/pj4dev
 *
 */

#include <unistd.h>

#include <nogdb/nogdb.h>


using namespace std;
using namespace nogdb;

int main(int argc, char* argv[]) {

  try {
    auto ctxi = ContextInitializer("example.db");
    auto ctx = ctxi.init();

    auto txn = std::move(ctx.beginTxn(TxnMode::READ_WRITE));
    txn.addClass("Words", ClassType::VERTEX);
    txn.addSubClassOf("Words", "InitialWords");
    txn.addClass("WordLinks", ClassType::EDGE);
    txn.addProperty("Words", "messaged", PropertyType::TEXT);

    auto vHello = txn.addVertex("InitialWords", Record().set("messaged", "Hello"));
    auto vWorld = txn.addVertex("Words", Record().set("messaged", "World."));
    txn.addEdge("WordLinks", vHello, vWorld);

    txn.commit();

    txn = ctx.beginTxn(TxnMode::READ_ONLY);
    auto rss = txn.find("InitialWords").get();
    auto initialWord = rss[0].record.getText("messaged");
    auto link = txn.findOutEdge(rss[0].descriptor).get()[0];
    auto word = txn.fetchDst(link.descriptor).record.getText("messaged");

    std::cout << initialWord << ", " << word << "\n";

  } catch (const nogdb::Error& err) {
    std::cerr << err.what() << "\n";
  }

  return 0;

}