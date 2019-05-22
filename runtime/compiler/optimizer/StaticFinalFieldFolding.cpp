/*******************************************************************************
 * Copyright (c) 2019, 2019 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include "optimizer/StaticFinalFieldFolding.hpp"
#include <stddef.h>
#include <stdint.h>
#include "codegen/CodeGenerator.hpp"
#include "codegen/FrontEnd.hpp"
#include "compile/Compilation.hpp"
#include "compile/ResolvedMethod.hpp"
#include "compile/SymbolReferenceTable.hpp"
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"
#include "env/CompilerEnv.hpp"
#include "env/IO.hpp"
#include "env/VMJ9.h"
#include "env/j9method.h"
#include "il/ILOpCodes.hpp"
#include "il/ILOps.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "il/Symbol.hpp"
#include "il/SymbolReference.hpp"
#include "il/TreeTop.hpp"
#include "il/TreeTop_inlines.hpp"
#include "il/symbol/MethodSymbol.hpp"
#include "il/symbol/ResolvedMethodSymbol.hpp"
#include "infra/Assert.hpp"
#include "infra/Checklist.hpp"
#include "optimizer/Optimization.hpp"
#include "optimizer/Optimization_inlines.hpp"
#include "optimizer/J9TransformUtil.hpp"

void TR_StaticFinalFieldFolding::visitNode(TR::TreeTop * currentTree, TR::Node *node)
   {
   if (!_checklist->contains(node))
      {
      uint16_t childCount = node->getNumChildren();
      if (0 == childCount)
         {
         if (node->getOpCode().isLoadVarDirect() && node->isLoadOfStaticFinalField())
            {
            J9::TransformUtil::attemptGenericStaticFinalFieldFolding(this, currentTree, node);
            }
         _checklist->add(node);
         return;
         }
      for (int i = childCount; i>0; i--)
         {
         visitNode(currentTree, node->getChild(i-1));
         }
      _checklist->add(node);
      }
   }

/**
 *
 * Fold static final fields based on certain criteria
 */
int32_t TR_StaticFinalFieldFolding::perform()
   {
   _checklist = new (trStackMemory()) TR::NodeChecklist(comp());

   TR::ResolvedMethodSymbol *methodSymbol = comp()->getMethodSymbol();
   TR_J9VMBase *fej9 = (TR_J9VMBase *)(comp()->fe());
   for (TR::TreeTop * tt = methodSymbol->getFirstTreeTop(); tt != NULL; tt = tt->getNextTreeTop())
      {
      TR::Node *node = tt->getNode(); // Get the first child of the tree
      TR_ASSERT_FATAL(node, "TT node cannot be null");
      visitNode(tt, node);
      }
   return 0;
   }

const char *
TR_StaticFinalFieldFolding::optDetailString() const throw()
   {
   return "O^O STATIC FINAL FIELD FOLDING: ";
   }
