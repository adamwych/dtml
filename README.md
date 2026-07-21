# DTML

DynamicText Markup Language (DTML) is a superset of HTML with additional features that help create dynamic content.

DTML is a **templating language**, it does not provide tools to create interactive content (you still use JavaScript for buttons, popovers, etc.).

## Development

To build and test take a look at the `scripts` directory.

## Example

```html
<html lang="en">
    <head>
        <meta charset="utf-8" />
        <title>Hello, world!</title>
    </head>
    <body>
        <!-- Fetch resource from `src` and store it in `author` variable. -->
        <partial src="https://jsonplaceholder.typicode.com/users/{todo.userId}" as="author" />

        <!-- You can now use `author` to create dynamic content! -->
        <div>{author.name}</div>

        <script>
            // You can also use `author` in runtime JavaScript, it's available as a global variable.
            console.log(author.name)
        </script>

        <repeat foreach="todo" in="https://jsonplaceholder.typicode.com/todos">
            <p>{todo.title}</p>
            <button>Delete</button>
        </repeat>
    </body>
</html>
```

## Elements

### `{expr}`

#### Example usage:

```html
<!-- Access members of an object -->
<h1>{todo.title}</h1>

<!-- String interpolation -->
<a href="https://example.com/todos/${todo.id}"></a>
```

---

### `<repeat>`

The `<repeat>` element fetches an array from address specified by the `src` attribute and repeats a block of code for every element in that list.

#### Example usage:

```html
<repeat foreach="todo" in="https://jsonplaceholder.typicode.com/todos">
    <h1>{todo.title}</h1>
</repeat>
```

## Credits

DTML uses the following third party libraries:

- [jsmn](https://github.com/zserge/jsmn)
- [utfcpp](https://github.com/nemtrif/utfcpp)

## License

MIT.
