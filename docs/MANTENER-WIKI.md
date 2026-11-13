# Mantener la wiki

Las páginas publicadas en la pestaña Wiki tienen una copia de sus fuentes en `docs/wiki/`, incluida la portada, el menú lateral y el pie. GitHub guarda la wiki en un repositorio separado; actualizar esta carpeta por sí sola no cambia la pestaña Wiki.

Después de editar y guardar las páginas de `docs/wiki/`, clona la wiki en una carpeta hermana si aún no la tienes:

```bash
git clone https://github.com/a01735995-sketch/Transformaciones.wiki.git ../Transformaciones.wiki
```

Desde la raíz del proyecto, copia las páginas y publica con una cuenta que tenga permiso de escritura.

Linux/macOS:

```bash
cp docs/wiki/*.md ../Transformaciones.wiki/
git -C ../Transformaciones.wiki add .
git -C ../Transformaciones.wiki commit -m "Actualizar documentacion de Transformaciones"
git -C ../Transformaciones.wiki push
```

PowerShell:

```powershell
Copy-Item docs\wiki\*.md ..\Transformaciones.wiki\
git -C ..\Transformaciones.wiki add .
git -C ..\Transformaciones.wiki commit -m "Actualizar documentacion de Transformaciones"
git -C ..\Transformaciones.wiki push
```

Conserva los nombres de archivo para mantener los enlaces. Verifica la portada y las páginas enlazadas después de publicar. [Documentación oficial sobre edición de wikis](https://docs.github.com/en/communities/documenting-your-project-with-wikis/adding-or-editing-wiki-pages).
