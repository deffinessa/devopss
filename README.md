# maxint

Minimal C CLI application that returns the maximum value from integers.

## Build

```bash
make
```

## Run

```bash
./maxint
# Array: ...
# Max: ...
```

## Package (.deb)

```bash
make package
```

The package can then be installed with:

```bash
sudo dpkg -i ../maxint_0.1.0-1_*.deb
```

## GitHub CI/CD

Pipeline is defined in `.github/workflows/ci.yml` and includes:
- build
- package artifact
- release asset on tags `v*`

## Publish to GitHub

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin <your_github_repo_url>
git push -u origin main
```

Create a release tag:

```bash
git tag v0.1.0
git push origin v0.1.0
```
