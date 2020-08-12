

.. raw:: html

   <!--===- documentation/OpenMP-semantics.rst 

      Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
      See https://llvm.org/LICENSE.txt for license information.
      SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

   -->



OpenMP Semantic Analysis
========================

OpenMP for F18
--------------


#. Define and document the parse tree representation for

   * Directives (listed below)
   * Clauses (listed below)
   * Documentation

#. All the directives and clauses need source provenance for messages
#. Define and document how an OpenMP directive in the parse tree
   will be represented as the parent of the statement(s)
   to which the directive applies.
   The parser itself will not be able to construct this representation;
   there will be subsequent passes that do so
   just like for example *do-stmt* and *do-construct*.
#. Define and document the symbol table extensions
#. Define and document the module file extensions

Directives
^^^^^^^^^^

OpenMP divides directives into three categories as follows.
The directives that are in the same categories share some characteristics.

Declarative directives
~~~~~~~~~~~~~~~~~~~~~~

An OpenMP directive may only be placed in a declarative context.
A declarative directive results in one or more declarations only;
it is not associated with the immediate execution of any user code.

List of existing ones:


* declare simd
* declare target
* threadprivate
* declare reduction

There is a parser node for each of these directives and
the parser node saves information associated with the directive,
for example,
the name of the procedure-name in the ``declare simd`` directive.

Each parse tree node keeps source provenance,
one for the directive name itself and
one for the entire directive starting from the directive name.

A top-level class, ``OpenMPDeclarativeConstruct``\ ,
holds all four of the node types as discriminated unions
along with the source provenance for the entire directive
starting from ``!$OMP``.

In ``parser-tree.h``\ ,
``OpenMPDeclarativeConstruct`` is part
of the ``SpecificationConstruct`` and ``SpecificationPart``
in F18 because
a declarative directive can only be placed in the specification part
of a Fortran program.

All the ``Names`` or ``Designators`` associated
with the declarative directive will be resolved in later phases.

Executable directives
~~~~~~~~~~~~~~~~~~~~~

An OpenMP directive that is **not** declarative.
That is, it may only be placed in an executable context.
It contains stand-alone directives and constructs
that are associated with code blocks.
The stand-alone directive is described in the next section.

The constructs associated with code blocks listed below
share a similar structure:
*Begin Directive*\ , *Clause List*\ , *Code Block*\ , *End Directive*.
The *End Directive* is optional for constructs
like Loop-associated constructs.


* Block-associated constructs (\ ``OpenMPBlockConstruct``\ )
* Loop-associated constructs (\ ``OpenMPLoopConstruct``\ )
* Atomic construct (\ ``OpenMPAtomicConstruct``\ )
* Sections Construct (\ ``OpenMPSectionsConstruct``\ ,
  contains Sections/Parallel Sections constructs)
* Critical Construct (\ ``OpenMPCriticalConstruct``\ )

A top-level class, ``OpenMPConstruct``\ ,
includes stand-alone directive and constructs
listed above as discriminated unions.

In the ``parse-tree.h``\ , ``OpenMPConstruct`` is an element
of the ``ExecutableConstruct``.

All the ``Names`` or ``Designators`` associated
with the executable directive will be resolved in Semantic Analysis.

When the backtracking parser can not identify the associated code blocks,
the parse tree will be rewritten later in the Semantics Analysis.

Stand-alone Directives
~~~~~~~~~~~~~~~~~~~~~~

An OpenMP executable directive that has no associated user code
except for that which appears in clauses in the directive.

List of existing ones:


* taskyield
* barrier
* taskwait
* target enter data
* target exit data
* target update
* ordered
* flush
* cancel
* cancellation point

A higher-level class is created for each category
which contains directives listed above that share a similar structure:


* OpenMPSimpleStandaloneConstruct
  (taskyield, barrier, taskwait,
  target enter/exit data, target update, ordered)
* OpenMPFlushConstruct
* OpenMPCancelConstruct
* OpenMPCancellationPointConstruct

A top-level class, ``OpenMPStandaloneConstruct``\ ,
holds all four of the node types as discriminated unions
along with the source provenance for the entire directive.
Also, each parser node for the stand-alone directive saves
the source provenance for the directive name itself.

Clauses
^^^^^^^

Each clause represented as a distinct class in ``parse-tree.h``.
A top-level class, ``OmpClause``\ ,
includes all the clauses as discriminated unions.
The parser node for ``OmpClause`` saves the source provenance
for the entire clause.

All the ``Names`` or ``Designators`` associated
with the clauses will be resolved in Semantic Analysis.

Note that the backtracking parser will not validate
that the list of clauses associated
with a directive is valid other than to make sure they are well-formed.
In particular,
the parser does not check that
the association between directive and clauses is correct
nor check that the values in the directives or clauses are correct.
These checks are deferred to later phases of semantics to simplify the parser.

Symbol Table Extensions for OpenMP
----------------------------------

Name resolution can be impacted by the OpenMP code.
In addition to the regular steps to do the name resolution,
new scopes and symbols may need to be created
when encountering certain OpenMP constructs.
This section describes the extensions
for OpenMP during Symbol Table construction.

OpenMP uses the fork-join model of parallel execution and
all OpenMP threads have access to
a *shared* memory place to store and retrieve variables
but each thread can also have access to
its *threadprivate* memory that must not be accessed by other threads.

For the directives and clauses that can control the data environments,
compiler needs to determine two kinds of *access*
to variables used in the directive’s associated structured block:
**shared** and **private**.
Each variable referenced in the structured block
has an original variable immediately outside of the OpenMP constructs.
Reference to a shared variable in the structured block
becomes a reference to the original variable.
However, each private variable referenced in the structured block,
a new version of the original variable (of the same type and size)
will be created in the threadprivate memory.

There are exceptions that directives/clauses
need to create a new ``Symbol`` without creating a new ``Scope``\ ,
but in general,
when encountering each of the data environment controlling directives
(discussed in the following sections),
a new ``Scope`` will be created.
For each private variable referenced in the structured block,
a new ``Symbol`` is created out of the original variable
and the new ``Symbol`` is associated
with original variable’s ``Symbol`` via ``HostAssocDetails``.
A new set of OpenMP specific flags are added
into ``Flag`` class in ``symbol.h`` to indicate the types of
associations,
data-sharing attributes,
and data-mapping attributes
in the OpenMP data environments.

New Symbol without new Scope
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

OpenMP directives that require new ``Symbol`` to be created
but not new ``Scope`` are listed in the following table
in terms of the Symbol Table extensions for OpenMP:


.. raw:: html

   <table>
     <tr>
      <td rowspan="2" colspan="2" >Directives/Clauses
      </td>
      <td rowspan="2" >Create New
   <p>
   Symbol
   <p>
   w/
      </td>
      <td colspan="2" >Add Flag
      </td>
     </tr>
     <tr>
      <td>on Symbol of
      </td>
      <td>Flag
      </td>
     </tr>
     <tr>
      <td rowspan="4" >Declarative Directives
      </td>
      <td>declare simd [(proc-name)]
      </td>
      <td>-
      </td>
      <td>The name of the enclosing function, subroutine, or interface body
      to which it applies, or proc-name
      </td>
      <td>OmpDeclareSimd
      </td>
     </tr>
     <tr>
      <td>declare target
      </td>
      <td>-
      </td>
      <td>The name of the enclosing function, subroutine, or interface body
      to which it applies
      </td>
      <td>OmpDeclareTarget
      </td>
     </tr>
     <tr>
      <td>threadprivate(list)
      </td>
      <td>-
      </td>
      <td>named variables and named common blocks
      </td>
      <td>OmpThreadPrivate
      </td>
     </tr>
     <tr>
      <td>declare reduction
      </td>
      <td>*
      </td>
      <td>reduction-identifier
      </td>
      <td>OmpDeclareReduction
      </td>
     </tr>
     <tr>
      <td>Stand-alone directives
      </td>
      <td>flush
      </td>
      <td>-
      </td>
      <td>variable, array section or common block name
      </td>
      <td>OmpFlushed
      </td>
     </tr>
     <tr>
      <td colspan="2" >critical [(name)]
      </td>
      <td>-
      </td>
      <td>name (user-defined identifier)
      </td>
      <td>OmpCriticalLock
      </td>
     </tr>
     <tr>
      <td colspan="2" >if ([ directive-name-modifier :] scalar-logical-expr)
      </td>
      <td>-
      </td>
      <td>directive-name-modifier
      </td>
      <td>OmpIfSpecified
      </td>
     </tr>
   </table>


.. code-block::

     -      No Action

     *      Discussed in “Module File Extensions for OpenMP” section



New Symbol with new Scope
^^^^^^^^^^^^^^^^^^^^^^^^^

For the following OpenMP regions:


* ``target`` regions
* ``teams`` regions
* ``parallel`` regions
* ``simd`` regions
* task generating regions (created by ``task`` or ``taskloop`` constructs)
* worksharing regions
  (created by ``do``\ , ``sections``\ , ``single``\ , or ``workshare`` constructs)

A new ``Scope`` will be created
when encountering the above OpenMP constructs
to ensure the correct data environment during the Code Generation.
To determine whether a variable referenced in these regions
needs the creation of a new ``Symbol``\ ,
all the data-sharing attribute rules
described in OpenMP Spec [2.15.1] apply during the Name Resolution.
The available data-sharing attributes are:
**\ *shared*\ **\ ,
**\ *private*\ **\ ,
**\ *linear*\ **\ ,
**\ *firstprivate*\ **\ ,
and **\ *lastprivate*\ **.
The attribute is represented as ``Flag`` in the ``Symbol`` object.

More details are listed in the following table:


.. raw:: html

   <table>
     <tr>
      <td rowspan="2" >Attribute
      </td>
      <td rowspan="2" >Create New Symbol
      </td>
      <td colspan="2" >Add Flag
      </td>
     </tr>
     <tr>
      <td>on Symbol of
      </td>
      <td>Flag
      </td>
     </tr>
     <tr>
      <td>shared
      </td>
      <td>No
      </td>
      <td>Original variable
      </td>
      <td>OmpShared
      </td>
     </tr>
     <tr>
      <td>private
      </td>
      <td>Yes
      </td>
      <td>New Symbol
      </td>
      <td>OmpPrivate
      </td>
     </tr>
     <tr>
      <td>linear
      </td>
      <td>Yes
      </td>
      <td>New Symbol
      </td>
      <td>OmpLinear
      </td>
     </tr>
     <tr>
      <td>firstprivate
      </td>
      <td>Yes
      </td>
      <td>New Symbol
      </td>
      <td>OmpFirstPrivate
      </td>
     </tr>
     <tr>
      <td>lastprivate
      </td>
      <td>Yes
      </td>
      <td>New Symbol
      </td>
      <td>OmpLastPrivate
      </td>
     </tr>
   </table>


To determine the right data-sharing attribute,
OpenMP defines that the data-sharing attributes
of variables that are referenced in a construct can be
*predetermined*\ , *explicitly determined*\ , or *implicitly determined*.

Predetermined data-sharing attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


* Assumed-size arrays are **shared**
* The loop iteration variable(s)
  in the associated *do-loop(s)* of a
  *do*\ ,
  *parallel do*\ ,
  *taskloop*\ ,
  or *distributeconstruct*
  is (are) **private**
* A loop iteration variable
  for a sequential loop in a *parallel* or task generating construct
  is **private** in the innermost such construct that encloses the loop
* Implied-do indices and *forall* indices are **private**
* The loop iteration variable in the associated *do-loop*
  of a *simd* construct with just one associated *do-loop*
  is **linear** with a linear-step
  that is the increment of the associated *do-loop*
* The loop iteration variables in the associated *do-loop(s)* of a *simd*
  construct with multiple associated *do-loop(s)* are **lastprivate**

Explicitly determined data-sharing attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Variables with *explicitly determined* data-sharing attributes are:


* Variables are referenced in a given construct
* Variables are listed in a data-sharing attribute clause on the construct.

The data-sharing attribute clauses are:


* *default* clause
  (discussed in “Implicitly determined data-sharing attributes”)
* *shared* clause
* *private* clause
* *linear* clause
* *firstprivate* clause
* *lastprivate* clause
* *reduction* clause
  (new ``Symbol`` created with the flag ``OmpReduction`` set)

Note that variables with *predetermined* data-sharing attributes
may not be listed (with exceptions) in data-sharing attribute clauses.

Implicitly determined data-sharing attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Variables with implicitly determined data-sharing attributes are:


* Variables are referenced in a given construct
* Variables do not have *predetermined* data-sharing attributes
* Variables are not listed in a data-sharing attribute clause
  on the construct.

Rules for variables with *implicitly determined* data-sharing attributes:


* In a *parallel* construct, if no *default* clause is present,
  these variables are **shared**
* In a task generating construct,
  if no *default* clause is present,
  a variable for which the data-sharing attribute
  is not determined by the rules above
  and that in the enclosing context is determined
  to be shared by all implicit tasks
  bound to the current team is **shared**
* In a *target* construct,
  variables that are not mapped after applying data-mapping attribute rules
  (discussed later) are **firstprivate**
* In an orphaned task generating construct,
  if no *default* clause is present, dummy arguments are **firstprivate**
* In a task generating construct, if no *default* clause is present,
  a variable for which the data-sharing attribute is not determined
  by the rules above is **firstprivate**
* For constructs other than task generating constructs or *target* constructs,
  if no *default* clause is present,
  these variables reference the variables with the same names
  that exist in the enclosing context
* In a *parallel*\ , *teams*\ , or task generating construct,
  the data-sharing attributes of these variables are determined
  by the *default* clause, if present:

  * *default(shared)*
    clause causes all variables referenced in the construct
    that have *implicitly determined* data-sharing attributes
    to be **shared**
  * *default(private)*
    clause causes all variables referenced in the construct
    that have *implicitly determined* data-sharing attributes
    to be **private**
  * *default(firstprivate)*
    clause causes all variables referenced in the construct
    that have *implicitly determined* data-sharing attributes
    to be **firstprivate**
  * *default(none)*
    clause requires that each variable
    that is referenced in the construct,
    and that does not have a *predetermined* data-sharing attribute,
    must have its data-sharing attribute *explicitly determined*
    by being listed in a data-sharing attribute clause

Data-mapping Attribute
^^^^^^^^^^^^^^^^^^^^^^

When encountering the *target data* and *target* directives,
the data-mapping attributes of any variable referenced in a target region
will be determined and represented as ``Flag`` in the ``Symbol`` object
of the variable.
No ``Symbol`` or ``Scope`` will be created.

The basic steps to determine the data-mapping attribute are:


#. If *map* clause is present,
   the data-mapping attribute is determined by the *map-type*
   on the clause and its corresponding ``Flag`` are listed below:


.. raw:: html

   <table>
     <tr>
      <td>
   data-mapping attribute
      </td>
      <td>Flag
      </td>
     </tr>
     <tr>
      <td>to
      </td>
      <td>OmpMapTo
      </td>
     </tr>
     <tr>
      <td>from
      </td>
      <td>OmpMapFrom
      </td>
     </tr>
     <tr>
      <td>tofrom
   (default if map-type is not present)
      </td>
      <td>OmpMapTo & OmpMapFrom
      </td>
     </tr>
     <tr>
      <td>alloc
      </td>
      <td>OmpMapAlloc
      </td>
     </tr>
     <tr>
      <td>release
      </td>
      <td>OmpMapRelease
      </td>
     </tr>
     <tr>
      <td>delete
      </td>
      <td>OmpMapDelete
      </td>
     </tr>
   </table>



#. Otherwise, the following data-mapping rules apply
   for variables referenced in a *target* construct
   that are *not* declared in the construct and
   do not appear in data-sharing attribute or map clauses:

   * If a variable appears in a *to* or *link* clause
     on a *declare target* directive then it is treated
     as if it had appeared in a *map* clause with a *map-type* of **tofrom**

#. Otherwise, the following implicit data-mapping attribute rules apply:

   * If a *defaultmap(tofrom:scalar)* clause is *not* present
     then a scalar variable is not mapped,
     but instead has an implicit data-sharing attribute of **firstprivate**
   * If a *defaultmap(tofrom:scalar)* clause is present
     then a scalar variable is treated as if it had appeared
     in a map clause with a map-type of **tofrom**
   * If a variable is not a scalar
     then it is treated as if it had appeared in a map clause
     with a *map-type* of **tofrom**

After the completion of the Name Resolution phase,
all the data-sharing or data-mapping attributes marked for the ``Symbols``
may be used later in the Semantics Analysis and in the Code Generation.

Module File Extensions for OpenMP
---------------------------------

After the successful compilation of modules and submodules
that may contain the following Declarative Directives,
the entire directive starting from ``!$OMP`` needs to be written out
into ``.mod`` files in their corresponding Specification Part:


* 
  *declare simd* or *declare target*

    In the “New Symbol without new Scope” section,
    we described that when encountering these two declarative directives,
    new ``Flag`` will be applied to the Symbol of the name of
    the enclosing function, subroutine, or interface body to
    which it applies, or proc-name.
    This ``Flag`` should be part of the API information
    for the given subroutine or function

* 
  *declare reduction*

    The *reduction-identifier* in this directive
    can be use-associated or host-associated.
    However, it will not act like other Symbols
    because user may have a reduction name
    that is the same as a Fortran entity name in the same scope.
    Therefore a specific data structure needs to be created
    to save the *reduction-identifier* information
    in the Scope and this directive needs to be written into ``.mod`` files

Phases of OpenMP Analysis
-------------------------


#. Create the parse tree for OpenMP

   #. Add types for directives and clauses

      #. Add type(s) that will be used for directives
      #. Add type(s) that will be used for clauses
      #. Add other types, e.g. wrappers or other containers
      #. Use std::variant to encapsulate meaningful types

   #. Implemented in the parser for OpenMP (openmp-grammar.h)

#. Create canonical nesting

   #. Restructure parse tree to reflect the association
      of directives and stmts

      #. Associate ``OpenMPLoopConstruct``
         with ``DoConstruct`` and ``OpenMPEndLoopDirective``

   #. Investigate, and perhaps reuse,
      the algorithm used to restructure do-loops
   #. Add a pass near the code that restructures do-loops;
      but do not extend the code that handles do-loop for OpenMP;
      keep this code separate.
   #. Report errors that prevent restructuring
      (e.g. loop directive not followed by loop)
      We should abort in case of errors
      because there is no point to perform further checks
      if it is not a legal OpenMP construct

#. Validate the structured-block

   #. Structured-block is a block of executable statements
   #. Single entry and single exit
   #. Access to the structured block must not be the result of a branch
   #. The point of exit cannot be a branch out of the structured block

#. Check that directive and clause combinations are legal

   #. Begin and End directive should match
   #. Simply check that the clauses are allowed by the directives
   #. Write as a separate pass for simplicity and correctness of the parse tree

#. Write parse tree tests

   #. At this point, the parse tree should be perfectly formed
   #. Write tests that check for correct form and provenance information
   #. Write tests for errors that can occur during the restructuring

#. Scope, symbol tables, and name resolution

   #. Update the existing code to handle names and scopes introduced by OpenMP
   #. Write tests to make sure names are properly implemented

#. Check semantics that is specific to each directive

   #. Validate the directive and its clauses
   #. Some clause checks require the result of name resolution,
      i.e. “A list item may appear in a *linear* or *firstprivate* clause
      but not both.”
   #. TBD:
      Validate the nested statement for legality in the scope of the directive
   #. Check the nesting of regions [OpenMP 4.5 spec 2.17]

#. Module file utilities

   #. Write necessary OpenMP declarative directives to ``.mod`` files
   #. Update the existing code
      to read available OpenMP directives from the ``.mod`` files
