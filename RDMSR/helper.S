.global TinyReadMsr
.text
#------------------------------------------------------------------------------
#  UINT64
#  TinyReadMsr (
#    IN   UINT32  Index,  // rdi
#    )
#------------------------------------------------------------------------------
TinyReadMsr:
      mov    %rdi, %rcx
      rdmsr
      shl    $0x20,%rdx
      or     %rdx,%rax
      ret