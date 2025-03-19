from flask import Flask, request, send_file, render_template, abort
from werkzeug.utils import secure_filename
import os
import shutil
import subprocess

app = Flask(__name__)

# 设置文件上传的目录为当前工作目录
app.config['UPLOAD_FOLDER'] = f"{os.getcwd()}/workfold"

@app.route('/')
def index():
    # 渲染前端页面
    return render_template('index.html')

@app.route('/upload', methods=['POST'])

def upload_file():
    yaml_file = request.files.get('yaml_file')
    eds_files = request.files.getlist('eds_file[]')  # 获取文件列表

    if os.path.exists(app.config['UPLOAD_FOLDER']):
        # Remove the folder and all its contents
        shutil.rmtree(app.config['UPLOAD_FOLDER']) 

    if not os.path.exists(app.config['UPLOAD_FOLDER']):
        # If the folder does not exist, create it
        os.makedirs(app.config['UPLOAD_FOLDER'])

    if not yaml_file or not eds_files or len(eds_files) == 0:
        return "Missing files", 400
    
    if yaml_file.filename == '':
        return "No selected YAML file", 400
    
    if not all(allowed_file(f.filename, ['eds']) for f in eds_files):
        return "Invalid EDS file types", 400
    
    yaml_filename = secure_filename(yaml_file.filename)
    yaml_file_path = os.path.join(app.config['UPLOAD_FOLDER'], yaml_filename)
    yaml_file.save(yaml_file_path)
    
    eds_file_paths = []
    for eds_file in eds_files:
        if eds_file.filename != '' and allowed_file(eds_file.filename, ['eds']):
            eds_filename = secure_filename(eds_file.filename)
            eds_file_path = os.path.join(app.config['UPLOAD_FOLDER'], eds_filename)
            eds_file.save(eds_file_path)
            eds_file_paths.append(eds_file_path)
    

    command = f"cd {app.config['UPLOAD_FOLDER']} && dcfgen {yaml_filename} && hcxxd ./"

    try:
        process = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        error = process.stderr.decode().strip()
        output = process.stdout.decode().strip()

        # 假设处理后的文件名为 'output.h'
        generated_filename = 'output.h'  # 这应该是实际生成的文件名
        generated_file_path = os.path.join(app.config['UPLOAD_FOLDER'], generated_filename)

        return send_file(generated_file_path, as_attachment=True)
    except subprocess.CalledProcessError as e:
        error_message = e.stderr.decode().strip() if e.stderr else "Command failed without error message"
        # 返回状态码400（Bad Request），并附带错误信息
        abort(400, description=f"Command failed: {error_message}")
    except Exception as e:
        # 捕获其他所有异常，并返回500内部服务器错误状态码
        abort(500, description=str(e))

def allowed_file(filename, extensions):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in extensions

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8100)

