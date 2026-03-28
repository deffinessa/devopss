# maxint — DevOps Labs Project

Учебный проект по курсу DevOps. Простая программа на C, реализующая поиск максимального значения в массиве случайных целых чисел. Программа запускается как HTTP-сервер и экспортирует метрики в формате Prometheus.

---

## Содержание

1. [Обзор](#обзор)
2. [Структура проекта](#структура-проекта)
3. [Практическая работа 1 — Сборка и пакетирование](#практическая-работа-1)
4. [Практическая работа 2 — CI/CD](#практическая-работа-2)
5. [Практическая работа 3 — Docker](#практическая-работа-3)
6. [Практическая работа 4 — Kubernetes + Мониторинг](#практическая-работа-4)
7. [Эндпоинты HTTP](#эндпоинты-http)
8. [Метрики Prometheus](#метрики-prometheus)

---

## Обзор

**maxint** — HTTP-сервер на языке C (без внешних зависимостей):
- генерирует массив из 10 случайных чисел в диапазоне [-100, 100];
- вычисляет и возвращает максимальное значение;
- экспортирует метрики Prometheus на эндпоинте `/metrics`.

---

## Структура проекта

```
.
├── src/
│   └── main.c                          # Исходный код (HTTP-сервер)
├── scripts/
│   ├── build.sh                        # Сборка бинарника
│   ├── test.sh                         # Автоматические тесты
│   └── package.sh                      # Создание .deb пакета
├── debian/                             # Метаданные Debian-пакета
├── k8s/
│   ├── deployment.yaml                 # Deployment для maxint
│   ├── service.yaml                    # Service для maxint (NodePort)
│   ├── prometheus/
│   │   ├── configmap.yaml              # Конфигурация Prometheus
│   │   ├── deployment.yaml             # Deployment Prometheus
│   │   └── service.yaml                # Service Prometheus (NodePort)
│   └── grafana/
│       ├── datasource-configmap.yaml   # Datasource Prometheus
│       ├── dashboard-provider-configmap.yaml
│       ├── dashboard-json-configmap.yaml  # JSON дашборда
│       ├── deployment.yaml             # Deployment Grafana
│       └── service.yaml                # Service Grafana (NodePort)
├── Dockerfile                          # Многоэтапная сборка
├── Makefile                            # Сборка через make
└── .github/workflows/ci.yml            # GitHub Actions CI/CD
```

---

## Практическая работа 1

### Требования

```bash
sudo apt-get install build-essential dpkg-dev debhelper
```

### Локальная сборка

```bash
make all
./maxint        # запустит сервер на :8080
```

Проверка:
```bash
curl http://localhost:8080/compute
# Array: -42 17 5 -99 63 0 -7 88 -14 31
# Max: 88
```

### Запуск тестов

```bash
./scripts/test.sh
```

Скрипт запускает сервер, проверяет все эндпоинты и корректность вычисления максимума.

### Сборка Debian-пакета

```bash
make package
# создаёт maxint_*.deb в родительском каталоге
```

Установка на целевой системе:
```bash
sudo dpkg -i maxint_*.deb
maxint
```

---

## Практическая работа 2

### GitHub Actions Pipeline

Пайплайн состоит из 5 этапов:

| Этап      | Триггер        | Описание                                      |
|-----------|----------------|-----------------------------------------------|
| `build`   | push/PR        | Сборка бинарника через `make all`             |
| `test`    | после build    | Запуск сервера и проверка всех эндпоинтов     |
| `package` | после test     | Создание `.deb` пакета                        |
| `deploy`  | после package  | Сборка Docker-образа и публикация на теге `v*`|
| `release` | только тег `v*`| Создание GitHub Release с приложенным `.deb`  |

### Создание релиза

```bash
git tag v1.0.0
git push origin v1.0.0
```

После успешного пайплайна:
- Docker-образ публикуется в Docker Hub как `<username>/maxint:1.0.0` и `latest`
- GitHub Release создаётся автоматически с приложенным `.deb` файлом

### Секреты для Docker Hub

В настройках репозитория GitHub (`Settings → Secrets → Actions`) добавить:
- `DOCKER_USERNAME` — имя пользователя Docker Hub
- `DOCKER_PASSWORD` — токен доступа Docker Hub

---

## Практическая работа 3

### Сборка Docker-образа

Dockerfile использует многоэтапную сборку: компиляция происходит в контейнере Ubuntu, финальный образ содержит только бинарник.

```bash
docker build -t maxint:local .
```

### Запуск контейнера

```bash
docker run -d --name maxint -p 8080:8080 maxint:local
```

Проверка:
```bash
curl http://localhost:8080/healthz
# OK

curl http://localhost:8080/compute
# Array: 12 -5 78 ...
# Max: 78

curl http://localhost:8080/metrics
# # HELP maxint_requests_total Total HTTP requests received
# # TYPE maxint_requests_total counter
# maxint_requests_total 3
# ...
```

Остановка:
```bash
docker stop maxint && docker rm maxint
```

---

## Практическая работа 4

### Требования

- [kubectl](https://kubernetes.io/docs/tasks/tools/)
- [minikube](https://minikube.sigs.k8s.io/docs/start/) (или другой Kubernetes-кластер)

### Запуск кластера (minikube)

```bash
minikube start
```

### 1. Развёртывание приложения maxint

```bash
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/service.yaml
```

Проверка:
```bash
kubectl get pods
# NAME                       READY   STATUS    RESTARTS   AGE
# maxint-7d9f6b8c5d-xk2lp    1/1     Running   0          30s

kubectl get service maxint-service
# NAME             TYPE       CLUSTER-IP      EXTERNAL-IP   PORT(S)          AGE
# maxint-service   NodePort   10.96.144.201   <none>        8080:3xxxx/TCP   30s
```

Получение URL (minikube):
```bash
minikube service maxint-service --url
# http://192.168.49.2:3xxxx
```

Проверка приложения:
```bash
curl $(minikube service maxint-service --url)/healthz
# OK

curl $(minikube service maxint-service --url)/compute
# Array: -42 17 5 -99 63 0 -7 88 -14 31
# Max: 88
```

### 2. Развёртывание Prometheus

```bash
kubectl apply -f k8s/prometheus/configmap.yaml
kubectl apply -f k8s/prometheus/deployment.yaml
kubectl apply -f k8s/prometheus/service.yaml
```

Проверка:
```bash
kubectl get pods -l app=prometheus
# NAME                          READY   STATUS    RESTARTS   AGE
# prometheus-6d8f9b7c4d-m3np2   1/1     Running   0          45s
```

Открыть веб-интерфейс Prometheus:
```bash
minikube service prometheus-service --url
# http://192.168.49.2:3xxxx
```

В браузере перейти по URL и проверить статус скрейпинга:
`Status → Targets` — цель `maxint` должна быть в состоянии **UP**.

Примеры запросов в Prometheus:
```
maxint_up
maxint_last_max_value
rate(maxint_requests_total[1m])
rate(maxint_computations_total[1m])
```

### 3. Развёртывание Grafana

```bash
kubectl apply -f k8s/grafana/datasource-configmap.yaml
kubectl apply -f k8s/grafana/dashboard-provider-configmap.yaml
kubectl apply -f k8s/grafana/dashboard-json-configmap.yaml
kubectl apply -f k8s/grafana/deployment.yaml
kubectl apply -f k8s/grafana/service.yaml
```

Проверка:
```bash
kubectl get pods -l app=grafana
# NAME                       READY   STATUS    RESTARTS   AGE
# grafana-5f8c9d7b6d-p2wq1   1/1     Running   0          30s
```

Открыть Grafana:
```bash
minikube service grafana-service --url
# http://192.168.49.2:3xxxx
```

**Учётные данные:** `admin` / `admin123`

Дашборд `maxint Dashboard` загрузится автоматически через provisioning.

### 4. Развёртывание всего стека одной командой

```bash
kubectl apply -f k8s/deployment.yaml \
              -f k8s/service.yaml \
              -f k8s/prometheus/configmap.yaml \
              -f k8s/prometheus/deployment.yaml \
              -f k8s/prometheus/service.yaml \
              -f k8s/grafana/datasource-configmap.yaml \
              -f k8s/grafana/dashboard-provider-configmap.yaml \
              -f k8s/grafana/dashboard-json-configmap.yaml \
              -f k8s/grafana/deployment.yaml \
              -f k8s/grafana/service.yaml
```

### 5. Генерация нагрузки для визуализации метрик

```bash
MAXINT_URL=$(minikube service maxint-service --url)

# Послать 100 запросов к /compute
for i in $(seq 1 100); do
  curl -s "${MAXINT_URL}/compute" > /dev/null
done

# Посмотреть метрики напрямую
curl -s "${MAXINT_URL}/metrics"
```

### 6. Удаление всех ресурсов

```bash
kubectl delete -f k8s/grafana/
kubectl delete -f k8s/prometheus/
kubectl delete -f k8s/service.yaml
kubectl delete -f k8s/deployment.yaml
```

---

## Эндпоинты HTTP

| Метод | Путь       | Описание                                  |
|-------|------------|-------------------------------------------|
| GET   | `/`        | Генерирует массив и возвращает максимум   |
| GET   | `/compute` | То же самое (явный эндпоинт)             |
| GET   | `/metrics` | Метрики в формате Prometheus              |
| GET   | `/healthz` | Проверка живости (`OK`)                   |

---

## Метрики Prometheus

| Метрика                        | Тип     | Описание                                      |
|--------------------------------|---------|-----------------------------------------------|
| `maxint_requests_total`        | counter | Общее количество HTTP-запросов                |
| `maxint_computations_total`    | counter | Количество выполненных вычислений максимума   |
| `maxint_last_max_value`        | gauge   | Последнее вычисленное максимальное значение   |
| `maxint_up`                    | gauge   | Доступность сервиса (1 = работает)            |

### Дашборд Grafana

Дашборд **maxint Dashboard** включает 5 панелей:

| Панель                  | Тип         | Запрос PromQL                           |
|-------------------------|-------------|------------------------------------------|
| HTTP Request Rate       | Time series | `rate(maxint_requests_total[1m])`        |
| Computation Rate        | Time series | `rate(maxint_computations_total[1m])`    |
| Last Max Value          | Gauge       | `maxint_last_max_value`                  |
| Service Status          | Stat        | `maxint_up`                              |
| Max Value Over Time     | Time series | `maxint_last_max_value`                  |
