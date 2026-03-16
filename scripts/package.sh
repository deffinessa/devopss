#!/usr/bin/env bash
set -euo pipefail

echo "=== Текущая директория ==="
pwd

echo "=== Содержимое родительской папки (../) ДО сборки ==="
ls -la ../ || echo "Родительская папка не доступна"

echo "=== Запуск make package ==="
make package

echo "=== Содержимое родительской папки (../) ПОСЛЕ сборки ==="
ls -la ../

echo "=== Попытка копирования deb-пакета ==="
cp -v ../*.deb ./maxint.deb || echo "Не удалось скопировать deb-пакет"

echo "=== Содержимое текущей папки после копирования ==="
ls -la ./maxint.deb || echo "Файл maxint.deb не найден"

echo "=== Скрипт package.sh завершён ==="
