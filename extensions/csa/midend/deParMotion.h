/*
 * Author: Hardik Soni
 * Email: hks57@cornell.edu
 */

#ifndef _EXTENSIONS_CSA_MIDEND_DEPARMERGE_H_ 
#define _EXTENSIONS_CSA_MIDEND_DEPARMERGE_H_ 

#include "ir/ir.h"
#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "frontends/p4/typeMap.h"

namespace CSA {

class DeParMerge final : public Transform {
    P4::ReferenceMap* refMap;
    P4::TypeMap* typeMap;
    const IR::P4ComposablePackage* cp2;

    bool deParMotion;
    IR::IndexedVector<IR::Type_Declaration> callees;

    bool isDeparser(const IR::P4Control* p4control);


    bool hasMultipleParsers(const IR::IndexedVector<IR::Type_Declaration>& cs);

    template<typename T> const T* 
      getCallee(const IR::IndexedVector<IR::Type_Declaration>& cs);

  public:
    using Transform::preorder;
    using Transform::postorder;

    explicit DeParMerge(P4::ReferenceMap* refMap, P4::TypeMap* typeMap) 
      : refMap(refMap), typeMap(typeMap) {
        CHECK_NULL(refMap); CHECK_NULL(typeMap);
        CHECK_NULL(cp2);
        setName("DeParMerge"); 
        deParMotion = false;
    }

    const IR::Node* preorder(IR::P4Parser* p4parser) override;

    const IR::Node* preorder(IR::P4Control* p4control) override;

    const IR::Node* preorder(IR::P4ComposablePackage* cp) override;

    const IR::Node* preorder(IR::IfStatement* ifstmt) override;
    const IR::Node* postorder(IR::IfStatement* ifstmt) override;

    const IR::Node* preorder(IR::BlockStatement* bs) override;
    const IR::Node* postorder(IR::BlockStatement* bs) override;

    const IR::Node* preorder(IR::MethodCallStatement* mcs) override;



};

class DeParMotion final : public PassRepeated {
    P4::ReferenceMap* refMap;
    P4::TypeMap* typeMap;
  public:
    explicit DeParMotion(P4::ReferenceMap* refMap, P4::TypeMap* typeMap)
      : PassManager({}), refMap(refMap), typeMap(typeMap) {
        passes.emplace_back(new P4::ResolveReferences(refMap, true));
        passes.emplace_back(new P4::TypeInference(refMap, typeMap, false));
    }
    void end_apply(const IR::Node* node) override {
        PassManager::end_apply(node);
    }
};


}   // namespace CSA
#endif  /* _EXTENSIONS_CSA_MIDEND_DEPARMERGE_H_ */

