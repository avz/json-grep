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
% cat log.json | json-grep type=earn
```

```
{"ts":1396141317.4536, "type":"earn", "amount":500, "user":{"id":1, "country":"RU"}}
{"ts":1396141318.5441, "type":"earn", "amount":50, "user":{"id":3, "country":"US", "ref": "fb"}}
{"ts":1396141319.7414, "type":"earn", "amount":10, "user":{"id":4, "country":"RU"}}
```

Второй уровень вложенности

```
% cat log.json | json-grep user.country=RU
```

```
{"ts":1396141317.4536, "type":"earn", "amount":500, "user":{"id":1, "country":"RU"}}
{"ts":1396141319.7414, "type":"earn", "amount":10, "user":{"id":4, "country":"RU"}}
```

### Существование поля

```
% cat log.json | json-grep user.ref
```

```
{"ts":1396141318.5441, "type":"earn", "amount":50, "user":{"id":3, "country":"US", "ref": "fb"}}
```

### Учёт значений произвольных полей

Если поле, по которому нужно фильтровать объекты, неизвестно, а известно только его значение,
то можно воспользоваться специальным символом `'*'` для обозначения любого поля. Вложенность
в данном случае тоже учитывается.

Для поиска только по первому уровню

```
% cat log.json | json-grep '*=50'
```

```
{"ts":1396141318.5441, "type":"earn", "amount":50, "user":{"id":3, "country":"US", "ref": "fb"}}
```

Для поиска только по второму уровню
```
% cat log.json | json-grep '*.*=RU'
```

```
{"ts":1396141317.4536, "type":"earn", "amount":500, "user":{"id":1, "country":"RU"}}
{"ts":1396141319.7414, "type":"earn", "amount":10, "user":{"id":4, "country":"RU"}}
```

## Обработка невалидного JSON

На настоящий момент невалидный JSON просто игнорируется и никогда не попадает в результат.
В некоторых случаях, парсер может посчитать невалидный JSON валидным, в этом случае результат
фильтрации невозможно предугадать.

## Ограничения

Значения сравниваются просто как строковое представления, поэтому по фильтру `"field:true"` будет
найден как объект `{"field": true}` так и объект `{"field": "true"}`, аналогично с `false` и `null`
и любыми числовыми значениями. Например, объект `{"field": 1e2}` будет заматчен только по фильтру
`field=1e2`, но не по фильтру `field=100`, как ожидается.

Кроме того, есть ограничение на максимальную длину JSON-представления объекта, сейчас длина
не должна превышать 64Кб.
