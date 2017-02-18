#pragma once

#include "SpriteBatch.h"
#include "Image.h"
#include "Drawable.h"

#include "gl/texture.h"

#include <memory>


namespace gameplay
{
    class ScreenOverlay : public Drawable
    {
    public:
        explicit ScreenOverlay(Game* game);
        ~ScreenOverlay();

        void resize();


        void draw(RenderContext& context) override;


        const std::shared_ptr<Image>& getImage() const
        {
            return _image;
        }


    private:

        ScreenOverlay(const ScreenOverlay& copy) = delete;
        ScreenOverlay& operator=(const ScreenOverlay&) = delete;

        std::shared_ptr<Image> _image{nullptr};
        std::shared_ptr<gl::Texture> _texture{nullptr};
        std::shared_ptr<Mesh> _mesh{nullptr};
        std::shared_ptr<Model> _model{nullptr};

        Game* _game;
    };
}
