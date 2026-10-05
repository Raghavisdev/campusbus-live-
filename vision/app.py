from flask import Flask, request, jsonify

app = Flask(__name__)

occupancy_data = {}


@app.route("/update-occupancy", methods=["POST"])
def update_occupancy():

    data = request.get_json()

    bus_id = data["busID"]
    occupancy = data["occupancy"]

    occupancy_data[bus_id] = occupancy

    print("Bus ID:", bus_id)
    print("Occupancy:", occupancy)

    return jsonify({
        "status": "success",
        "busID": bus_id,
        "occupancy": occupancy
    })


@app.route("/get-occupancy/<bus_id>", methods=["GET"])
def get_occupancy(bus_id):

    if bus_id not in occupancy_data:
        return jsonify({
            "status": "error",
            "message": "Bus not found"
        }), 404

    return jsonify({
        "busID": bus_id,
        "occupancy": occupancy_data[bus_id]
    })


@app.route("/health", methods=["GET"])
def health():

    return jsonify({
        "status": "running"
    })


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000)