#include "storage.h"

#include <fstream>
#include <limits>
#include <unordered_map>

namespace symmetrize {
namespace bdd {

using Size = uint64_t;
const Size IdConstant = std::numeric_limits<Size>::max();

struct Ref {
  bool inverted;
  Size id;
};

struct Entry {
  Size id;
  DdHalfWord index;
  Ref t;
  Ref e;
};

namespace write {

using EntryTable = std::unordered_map<DdNode *, Entry>;

static Ref GetRef(DdNode *node, EntryTable &table);

static Entry &EntryOf(DdNode *branch, EntryTable &table) {
  branch = Cudd_Regular(branch);
  auto it = table.find(branch);
  if (it != table.end())
    return it->second;
  Ref t = GetRef(Cudd_T(branch), table);
  Ref e = GetRef(Cudd_E(branch), table);
  return table.emplace(branch, Entry{table.size(), branch->index, t, e})
      .first->second;
}

static Ref GetRef(DdNode *node, EntryTable &table) {
  if (Cudd_IsConstant(node)) {
    return {Cudd_IsComplement(node) == 1, IdConstant};
  }
  Entry &entry = EntryOf(node, table);
  return Ref{Cudd_IsComplement(node) == 1, entry.id};
}

static void WriteSize(Size size, std::ostream &stream) {
  stream.write((const char *)&size, sizeof(Size));
}

static void WriteTable(EntryTable &table, std::ostream &stream) {
  WriteSize(table.size(), stream);
  for (auto &e : table) {
    Entry &entry = e.second;
    stream.write((const char *)&entry, sizeof(Entry));
  }
}

static void Write(const BDDs &bdd, std::ostream &stream) {
  DdManager *manager = bdd.GetManager();
  int vars = manager->size;
  stream.write((const char *)&vars, sizeof(vars));
  for (int i = 0; i < vars; i++) {
    int perm = Cudd_ReadInvPerm(manager, i);
    stream.write((const char *)&perm, sizeof(perm));
  }

  EntryTable table;
  for (auto &c : bdd.components)
    GetRef(c.Get(), table);
  WriteTable(table, stream);
  WriteSize(bdd.components.size(), stream);
  for (auto &c : bdd.components) {
    Ref ref = GetRef(c.Get(), table);
    stream.write((const char *)&ref, sizeof(Ref));
  }
}

} // namespace write

void Write(const BDDs &bdd, std::ostream &stream) { write::Write(bdd, stream); }

void Write(const BDDs &bdd, const std::string &filename) {
  std::ofstream stream(filename, std::ios::binary);
  stream.exceptions(std::ios::badbit);
  Write(bdd, stream);
  stream.close();
}

namespace read {

using EntryTable = std::unordered_map<Size, Entry>;
using NodeTable = std::unordered_map<Size, DdNode *>;

static Size ReadSize(std::istream &stream) {
  Size res;
  stream.read((char *)&res, sizeof(Size));
  return res;
}

static EntryTable ReadTable(std::istream &stream) {
  Size n = ReadSize(stream);
  EntryTable table;
  table.reserve(n);
  for (Size i = 0; i < n; i++) {
    Entry entry{};
    stream.read((char *)&entry, sizeof(Entry));
    table.emplace(entry.id, entry);
  }
  return table;
}

static DdNode *NodeFor(Ref ref, DdNode *node) {
  return ref.inverted ? Cudd_Complement(node) : node;
}

static DdNode *NodeFor(DdManager *manager, Ref ref, EntryTable &table,
                       NodeTable &nodes) {
  if (ref.id == IdConstant) {
    return NodeFor(ref, Cudd_ReadOne(manager));
  }
  auto it = nodes.find(ref.id);
  if (it != nodes.end())
    return NodeFor(ref, it->second);
  Entry &entry = table[ref.id];
  DdNode *t = NodeFor(manager, entry.t, table, nodes);
  DdNode *e = NodeFor(manager, entry.e, table, nodes);
  DdNode *var = Cudd_bddIthVar(manager, entry.index);
  return NodeFor(
      ref,
      nodes.emplace(ref.id, Cudd_bddIte(manager, var, t, e)).first->second);
}

static bdd::BDDs Read(std::istream &stream, DdManager *manager) {
  {
    int vars;
    stream.read((char *)&vars, sizeof(vars));
    std::vector<int> var_positions(vars);
    for (int i = 0; i < vars; i++) {
      stream.read((char *)&var_positions[i], sizeof(int));
    }
    if (manager == nullptr) {
      manager = Cudd_Init(vars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
      if (!Cudd_ShuffleHeap(manager, var_positions.data())) {
        throw std::logic_error("Cudd_ShuffleHeap failed.");
      }
    }
  }

  EntryTable table = ReadTable(stream);
  bdd::BDDs bdd;
  Size n = ReadSize(stream);
  bdd.components.reserve(n);
  NodeTable nodes;
  for (Size i = 0; i < n; i++) {
    Ref ref{};
    stream.read((char *)&ref, sizeof(Ref));
    bdd.components.emplace_back(
        BDD(manager, NodeFor(manager, ref, table, nodes)));
  }
  return bdd;
}

} // namespace read

bdd::BDDs Read(std::istream &stream, DdManager *manager) {
  return read::Read(stream, manager);
}

bdd::BDDs Read(const std::string &filename, DdManager *manager) {
  std::ifstream stream(filename, std::ios::binary);
  stream.exceptions(std::ios::badbit | std::ios::eofbit);
  bdd::BDDs bdd = Read(stream, manager);
  stream.close();
  return bdd;
}

} // namespace bdd
} // namespace symmetrize
