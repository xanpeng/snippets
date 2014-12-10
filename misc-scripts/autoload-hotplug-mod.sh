#/bin/sh
 
## Check hotplug modules, modprobe them and make them auto-loadable if needed.
##
## Check running kernel, determine if acpiphp & pci_hotplug is module,
## if so modprobe them and make them auto-loaded at boot time.
##
## How to run:
## 1. copy this shell into target host or vm
## 2. sh THIS_SHELL.sh
## 3. [optional] reboot to check
##
 
log_debug()
{
  echo "[DEBUG] $1"
}
 
log_error()
{
  echo "[ERROR] $1"
}
 
make_auto_load_suse()
{
  local modname=$1
  orig_content=`grep "^MODULES_LOADED_ON_BOOT" /etc/sysconfig/kernel`
  no_last_mark=${orig_content%\"}
  new_content="$no_last_mark $modname\""
  sed -i "s/$orig_content/$new_content/g" /etc/sysconfig/kernel
}
 
make_auto_load_rhel()
{
  local modname=$1
  modfile=/etc/sysconfig/modules/${modname}.modules
  touch $modfile
  chmod +x $modfile
  echo "#!/bin/sh" >> $modfile
  echo "exec /sbin/modprobe $modname >/dev/null 2>&1" >> $modfile
}
 
make_auto_load_ubuntu()
{
  local modname=$1
  echo $modname >> /etc/modules
}
 
make_auto_load()
{
  local modname=$1
 
  grep "SUSE" /etc/issue
  if [ $? -eq 0 ]; then
    make_auto_load_suse $modname
    return 0
  fi
 
  grep "Ubuntu" /etc/issue
  if [ $? -eq 0 ]; then
    make_auto_load_ubuntu $modname
    return 0
  fi
 
  grep "Red Hat" /etc/issue
  if [ $? -eq 0 ]; then
    make_auto_load_rhel $modname
    return 0
  fi
 
  log_error "OS not supported"
  return 1
}
 
 
#######
# main
#######
 
exit_fail=1
exit_success=0
 
compiled_in_kernel=100
compiled_in_mod=101
compiled_no=102
 
kernel_config=/boot/config-`uname -r`
 
pci_hotplug_as=$compiled_no
acpi_as=$compiled_no
modname_pci_hotplug=pci_hotplug
modname_acpi=acpiphp
 
# check
grep "CONFIG_HOTPLUG_PCI=y" $kernel_config
if [ $? -eq 0 ]; then
  pci_hotplug_as=$compiled_in_kernel
fi
grep "CONFIG_HOTPLUG_PCI=m" $kernel_config
if [ $? -eq 0 ]; then
  pci_hotplug_as=$compiled_in_mod
fi
 
grep "CONFIG_HOTPLUG_PCI_ACPI=y" $kernel_config
if [ $? -eq 0 ]; then
  acpi_as=$compiled_in_kernel
fi
grep "CONFIG_HOTPLUG_PCI_ACPI=m" $kernel_config
if [ $? -eq 0 ]; then
  acpi_as=$compiled_in_mod
fi
 
# take action
if [ $pci_hotplug_as -eq $compiled_in_mod ]; then
  log_debug "$modname_pci_hotplug is module, modprobe and make-auto-load"
  modprobe $modname_pci_hotplug
  if [ $? -ne 0 ]; then
    log_error "modprobe $modname_pci_hotplug failed"
    exit $exit_fail
  fi
  make_auto_load $modname_pci_hotplug
fi
 
if [ $acpi_as -eq $compiled_in_mod ]; then
  log_debug "$modname_acpi is module, modprobe and make-auto-load"
  modprobe $modname_acpi
  if [ $? -ne 0 ]; then
    log_error "modprobe $MODNAME_APCI failed"
    exit $exit_fail
  fi
  make_auto_load $modname_acpi
fi
 
# report non-support case
if [ $pci_hotplug_as -eq $compiled_no ]; then
  log_error "$modname_pci_hotplug is not supported"
  exit $exit_fail
fi
 
if [ $acpi_as -eq $compiled_no ]; then
  log_error "$modname_acpi is not support"
  exit $exit_fail
fi
 
exit $exit_success
