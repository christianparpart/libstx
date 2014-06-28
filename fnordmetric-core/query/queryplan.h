/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * Licensed under the MIT license (see LICENSE).
 */
#ifndef _FNORDMETRIC_QUERY_PLANER_H
#define _FNORDMETRIC_QUERY_PLANER_H
#include <stdlib.h>
#include <string>
#include <vector>
#include <assert.h>
#include "token.h"
#include "astnode.h"

namespace fnordmetric {
namespace query {
class Executable;
class TableRepository;

class QueryPlan {
public:

  /* Build a query plan for the provided SELECT staement */
  static Executable* buildQueryPlan(
      ASTNode* select_statement, TableRepository* repo);

protected:

  /**
   * Returns true if the ast is a SELECT statement that has a GROUP BY clause,
   * otherwise false
   */
  static bool hasGroupByClause(ASTNode* ast);

  /**
   * Returns true if the ast is a SELECT statement with a select list that
   * contains at least one aggregation expression, otherwise false.
   */
  static bool hasAggregationInSelectList(ASTNode* ast);

  /**
   * Walks the ast recursively and returns true if at least one aggregation
   * expression was found, otherwise false.
   */
  static bool hasAggregationExpression(ASTNode* ast);

  /**
   * Build a group by query plan node for a SELECT statement that has a GROUP
   * BY clause */
  static Executable* buildGroupBy(ASTNode* ast, TableRepository* repo);

  /**
   * Recursively walk the provided ast and search for column references. For
   * each found column reference, add the column reference to the provided
   * select list and replace the original column reference with an index into
   * the new select list.
   *
   * This is used to create child select lists for nested query plan nodes.
   */
  static bool buildInternalSelectList(ASTNode* ast, ASTNode* select_list);

};

}
}
#endif
