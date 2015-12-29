.PHONY: clean All

All:
	@echo "----------Building project:[ add - Debug ]----------"
	@cd "add" && "$(MAKE)" -f  "add.mk"
clean:
	@echo "----------Cleaning project:[ add - Debug ]----------"
	@cd "add" && "$(MAKE)" -f  "add.mk" clean
