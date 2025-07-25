// RUN: %clang_cc1 -triple thumbv7-apple-ios -Wno-objc-root-class -fsyntax-only -verify -Wformat %s
// RUN: %clang_cc1 -triple thumbv7-apple-ios -Wno-objc-root-class -fsyntax-only -verify -Wformat-pedantic -DPEDANTIC %s
// RUN: %clang_cc1 -triple thumbv7k-apple-watchos2.0.0 -fsyntax-only -fblocks -verify %s
// RUN: %clang_cc1 -triple thumbv7k-apple-watchos2.0.0 -fsyntax-only -fblocks -verify -Wformat-pedantic -DPEDANTIC %s

#if __LP64__
typedef unsigned long NSUInteger;
typedef long NSInteger;
typedef long ptrdiff_t;
#else
typedef unsigned int NSUInteger;
typedef int NSInteger;
#if __is_target_os(watchos)
  // Watch ABI uses long for ptrdiff_t.
  typedef long ptrdiff_t;
#else
  typedef int ptrdiff_t;
#endif
#endif

@class NSString;

extern void NSLog(NSString *format, ...);

void testSizeSpecifier(void) {
  NSInteger i = 0;
  NSUInteger j = 0;
  NSLog(@"max NSInteger = %zi", i);
#if defined(PEDANTIC)
  // expected-warning@-2 {{values of type 'NSInteger' should not be used as format arguments; add an explicit cast to 'long' instead}}
#endif
  NSLog(@"max NSUinteger = %zu", j); // expected-warning {{values of type 'NSUInteger' should not be used as format arguments; add an explicit cast to 'unsigned long' instead}}
}

void testPtrdiffSpecifier(ptrdiff_t x) {
  NSInteger i = 0;
  NSUInteger j = 0;

  NSLog(@"ptrdiff_t NSUinteger: %tu", j);
#if __is_target_os(watchos) && defined(PEDANTIC)
  // expected-warning@-2 {{values of type 'NSUInteger' should not be used as format arguments; add an explicit cast to 'unsigned long' instead}}
#endif
  NSLog(@"ptrdiff_t NSInteger: %td", i); // expected-warning {{values of type 'NSInteger' should not be used as format arguments; add an explicit cast to 'long' instead}}
  NSLog(@"ptrdiff_t %tu, %td", x, x); // no-warning
}
