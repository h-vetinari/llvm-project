.. title:: clang-tidy - cppcoreguidelines-avoid-goto

cppcoreguidelines-avoid-goto
============================

The usage of ``goto`` for control flow is error prone and should be replaced
with looping constructs. Only forward jumps in nested loops are accepted.

This check implements `ES.76
<https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es76-avoid-goto>`_
from the C++ Core Guidelines and
`6.3.1 <https://www.perforce.com/resources/qac/high-integrity-cpp-coding-standard/statements>`_
from High Integrity C++ Coding Standard.

For more information on why to avoid programming
with ``goto`` you can read the famous paper `A Case against the GO TO Statement. <https://www.cs.utexas.edu/users/EWD/ewd02xx/EWD215.PDF>`_.

The check diagnoses ``goto`` for backward jumps in every language mode. These
should be replaced with `C/C++` looping constructs.

.. code-block:: c++

  // Bad, handwritten for loop.
  int i = 0;
  // Jump label for the loop
  loop_start:
  do_some_operation();

  if (i < 100) {
    ++i;
    goto loop_start;
  }

  // Better
  for(int i = 0; i < 100; ++i)
    do_some_operation();

Modern C++ needs ``goto`` only to jump out of nested loops.

.. code-block:: c++

  for(int i = 0; i < 100; ++i) {
    for(int j = 0; j < 100; ++j) {
      if (i * j > 500)
        goto early_exit;
    }
  }

  early_exit:
  some_operation();

All other uses of ``goto`` are diagnosed in `C++`.


Options
-------

.. option:: IgnoreMacros

   If set to `true`, the check will not warn if a ``goto`` statement is
   expanded from a macro. Default is `false`.
