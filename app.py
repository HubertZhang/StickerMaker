import io
import json

from PIL import Image
from flask import Flask
from flask import abort, send_file
from flask import request

from sticker import generate_image

app = Flask(__name__)

config = json.load(open("data/config.json", encoding="utf-8"))
sub_configs = {
    x["name"]: x for x in config["sticker_config"]
}
for x in sub_configs:
    sub_configs[x]["source"] = "data/" + sub_configs[x]["source"]


@app.route("/<template_name>.jpg")
def handle(template_name):
    subconfig = sub_configs.get(template_name)
    if subconfig is None:
        abort(404)
    filename = '{}.jpg'.format(subconfig["name"])
    para = dict()
    if request.query_string == b"":
        return send_file("data/" + filename, mimetype="image/jpeg")

    for t in subconfig["texts"]:
        if request.args.get(t, None, type=str) is None:
            abort(404)
        para[t] = request.args.get(t, "", type=str)
    image = Image.open(generate_image(subconfig, para))
    rgb_im = image.convert('RGB')
    f = io.BytesIO()
    rgb_im.save(f, format='JPEG')
    f.seek(0)
    return send_file(f,
                     attachment_filename=filename,
                     mimetype="image/jpeg")


@app.route("/<template_name>.png")
def handlePNG(template_name):
    subconfig = sub_configs.get(template_name)
    if subconfig is None:
        abort(404)
    filename = '{}.png'.format(subconfig["name"])
    para = dict()
    if request.query_string == b"":
        return send_file("data/" + filename, mimetype="image/png")
    for t in subconfig["texts"]:
        if request.args.get(t, None, type=str) is None:
            abort(404)
        para[t] = request.args.get(t, "", type=str)
    f = generate_image(subconfig, para)
    return send_file(f,
                     attachment_filename=filename,
                     mimetype="image/png")


# @app.route("/rabbit.jpg")
# def rabbit():
#     subconfig = config["sticker_config"][0]
#     return handle_request(subconfig)
#
#
# @app.route("/rabbitc.jpg")
# def rabbitc():
#     subconfig = config["sticker_config"][1]
#     return handle_request(subconfig)


if __name__ == '__main__':
    # app.run(host="0.0.0.0", debug=True)
    app.run(host="0.0.0.0", port=8888)
