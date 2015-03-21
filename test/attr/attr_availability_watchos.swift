// RUN: %swift -parse -verify -parse-stdlib -target i386-apple-watchos3.0 %s

@availability(watchOS, introduced=1.0, deprecated=2.0, obsoleted=3.0,
              message="you don't want to do that anyway")
func doSomething() { }
// expected-note @-1{{'doSomething()' was obsoleted in watchOS 3.0}}

doSomething() // expected-error{{'doSomething()' is unavailable: you don't want to do that anyway}}

// Preservation of major.minor.micro
@availability(watchOS, introduced=1.0, deprecated=2.0, obsoleted=2.1.3)
func doSomethingElse() { }
// expected-note @-1{{'doSomethingElse()' was obsoleted in watchOS 2.1.3}}

doSomethingElse() // expected-error{{'doSomethingElse()' is unavailable}}

// Preservation of minor-only version
@availability(watchOS, introduced=1.0, deprecated=1.5, obsoleted=2)
func doSomethingReallyOld() { }
// expected-note @-1{{'doSomethingReallyOld()' was obsoleted in watchOS 2}}

doSomethingReallyOld() // expected-error{{'doSomethingReallyOld()' is unavailable}}

// Test deprecations in 2.0 and later

@availability(watchOS, introduced=1.1, deprecated=2.0,
              message="Use another function")
func deprecatedFunctionWithMessage() { }

deprecatedFunctionWithMessage() // expected-warning{{'deprecatedFunctionWithMessage()' was deprecated in watchOS 2.0: Use another function}}


@availability(watchOS, introduced=1.0, deprecated=2.0)
func deprecatedFunctionWithoutMessage() { }

deprecatedFunctionWithoutMessage() // expected-warning{{'deprecatedFunctionWithoutMessage()' was deprecated in watchOS 2.0}}

@availability(watchOS, introduced=1.0, deprecated=2.0,
              message="Use BetterClass instead")
class DeprecatedClass { }

func functionWithDeprecatedParameter(p: DeprecatedClass) { } // expected-warning{{'DeprecatedClass' was deprecated in watchOS 2.0: Use BetterClass instead}}

@availability(watchOS, introduced=2.0, deprecated=4.0,
              message="Use BetterClass instead")
class DeprecatedClassIn3_0 { }

// Elements deprecated later than the minimum deployment target (which is 3.0, in this case) should not generate warnings
func functionWithDeprecatedLaterParameter(p: DeprecatedClassIn3_0) { }