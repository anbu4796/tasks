cmd_/home/anbu/Documents/task_chat/Module.symvers := sed 's/\.ko$$/\.o/' /home/anbu/Documents/task_chat/modules.order | scripts/mod/modpost -m -a  -o /home/anbu/Documents/task_chat/Module.symvers -e -i Module.symvers   -T -