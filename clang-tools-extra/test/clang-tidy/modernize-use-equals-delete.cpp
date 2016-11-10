// RUN: %check_clang_tidy %s modernize-use-equals-delete %t

struct PositivePrivate {
private:
  PositivePrivate();
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositivePrivate() = delete;
  PositivePrivate(const PositivePrivate &);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositivePrivate(const PositivePrivate &) = delete;
  PositivePrivate &operator=(const PositivePrivate &);
  // CHECK-MESSAGES: :[[@LINE-1]]:20: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositivePrivate &operator=(const PositivePrivate &) = delete;
  PositivePrivate(PositivePrivate &&);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositivePrivate(PositivePrivate &&) = delete;
  PositivePrivate &operator=(PositivePrivate &&);
  // CHECK-MESSAGES: :[[@LINE-1]]:20: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositivePrivate &operator=(PositivePrivate &&) = delete;
  ~PositivePrivate();
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: ~PositivePrivate() = delete;
};

struct NegativePublic {
  NegativePublic(const NegativePublic &);
};

struct NegativeProtected {
protected:
  NegativeProtected(const NegativeProtected &);
};

struct PositiveInlineMember {
  int foo() { return 0; }

private:
  PositiveInlineMember(const PositiveInlineMember &);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositiveInlineMember(const PositiveInlineMember &) = delete;
};

struct PositiveOutOfLineMember {
  int foo();

private:
  PositiveOutOfLineMember(const PositiveOutOfLineMember &);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositiveOutOfLineMember(const PositiveOutOfLineMember &) = delete;
};

int PositiveOutOfLineMember::foo() { return 0; }

struct PositiveAbstractMember {
  virtual int foo() = 0;

private:
  PositiveAbstractMember(const PositiveAbstractMember &);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositiveAbstractMember(const PositiveAbstractMember &) = delete;
};

struct NegativeMemberNotImpl {
  int foo();

private:
  NegativeMemberNotImpl(const NegativeMemberNotImpl &);
};

struct NegativeStaticMemberNotImpl {
  static int foo();

private:
  NegativeStaticMemberNotImpl(const NegativeStaticMemberNotImpl &);
};

struct NegativeInline {
private:
  NegativeInline(const NegativeInline &) {}
};

struct NegativeOutOfLine {
private:
  NegativeOutOfLine(const NegativeOutOfLine &);
};

NegativeOutOfLine::NegativeOutOfLine(const NegativeOutOfLine &) {}

struct NegativeConstructNotImpl {
  NegativeConstructNotImpl();

private:
  NegativeConstructNotImpl(const NegativeConstructNotImpl &);
};

struct PositiveDefaultedConstruct {
  PositiveDefaultedConstruct() = default;

private:
  PositiveDefaultedConstruct(const PositiveDefaultedConstruct &);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositiveDefaultedConstruct(const PositiveDefaultedConstruct &) = delete;
};

struct PositiveDeletedConstruct {
  PositiveDeletedConstruct() = delete;

private:
  PositiveDeletedConstruct(const PositiveDeletedConstruct &);
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: use '= delete' to prohibit calling of a special member function [modernize-use-equals-delete]
  // CHECK-FIXES: PositiveDeletedConstruct(const PositiveDeletedConstruct &) = delete;
};

struct NegativeDefaulted {
private:
  NegativeDefaulted(const NegativeDefaulted &) = default;
};

struct PrivateDeleted {
private:
  PrivateDeleted(const PrivateDeleted &) = delete;
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: deleted member function should be public [modernize-use-equals-delete]
};

struct ProtectedDeleted {
protected:
  ProtectedDeleted(const ProtectedDeleted &) = delete;
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: deleted member function should be public [modernize-use-equals-delete]
};

struct PublicDeleted {
public:
  PublicDeleted(const PublicDeleted &) = delete;
};
