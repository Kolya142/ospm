# Open and Simple Package Menager

## install
```
# build ospm
gcc main.c -Os -o ospm
```
```
ospm init
ospm repo add https://raw.githubusercontent.com/Kolya142/ospm_main_repo/refs/heads/main/ospm.yaml
```

## check repo list
```
ospm check repo
```

## check package list
```
ospm check package <repo name>
```

## install package
```
ospm add <repo name> <package name>
```