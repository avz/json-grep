# json-grep

Быстрая реализация аналога утилиты `grep` для файлов, где каждая строка содержит
ровно один JSON-объект, например

```
% cat log.json
```

```
{"ts":1396141317.4536, "type":"earn", "amount":500, "user":{"id":1, "country":"RU"}}
{"ts":1396141317.6861, "type":"spent", "amount":100, "user":{"id":2, "country":"UA"}}
{"ts":1396141318.5441, "type":"earn", "amount":50, "user":{"id":3, "country":"US", "ref": "fb"}}
{"ts":1396141319.7414, "type":"earn", "amount":10, "user":{"id":4, "country":"RU"}}
```

## Фильтры

на данный момент поддерживаются 2 вида фильтрации: по точному совпадению значения
указанного поля и по существованию поля.

### Точное совпадение

Первый уровень пложенности

```
% cat log.json | jzon-grep type=earn
```

```
{"ts":1396141317.4536, "type":"earn", "amount":500, "user":{"id":1, "country":"RU"}}
{"ts":1396141318.5441, "type":"earn", "amount":50, "user":{"id":3, "country":"US", "ref": "fb"}}
{"ts":1396141319.7414, "type":"earn", "amount":10, "user":{"id":4, "country":"RU"}}
```

Второй уровень вложенности

```
% cat log.json | jzon-grep user.country=RU
```

```
{"ts":1396141317.4536, "type":"earn", "amount":500, "user":{"id":1, "country":"RU"}}
{"ts":1396141319.7414, "type":"earn", "amount":10, "user":{"id":4, "country":"RU"}}
```

### Существование поля

```
% cat log.json | jzon-grep user.ref
```

```
{"ts":1396141318.5441, "type":"earn", "amount":50, "user":{"id":3, "country":"US", "ref": "fb"}}
```
