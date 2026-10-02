#pragma once

#include "editor/components/cropWidget.h"
#include "editor/components/enumComboBox.h"
#include "editor/components/loadingSpinner.h"
#include "editor/texture/textureImportProcessor.h"

#include "ptcl/ptclEnum.h"
#include "ptcl/ptclTexture.h"
#include "util/imageUtil.h"

#include <QCheckBox>
#include <QDialog>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <atomic>

#include <memory>


// ========================================================================== //


class TextureImportDialog : public QDialog {
    Q_OBJECT
public:
    // Qt5's QFuture needs copyable results, so the unique_ptr is carried in a shared box.
    using TextureResult = std::shared_ptr<std::unique_ptr<Ptcl::Texture>>;

    explicit TextureImportDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~TextureImportDialog() override;

    void setImage(const QImage& image);
    std::unique_ptr<Ptcl::Texture> getTexture();

private slots:
    void updateTextureFormat();
    void updateAdjustment();

private:
    void updateImportPreview();
    void updateFormatPreview();
    void updateTextureInfo();
    void populateSizeCombo(QComboBox& combo, s32 initial);

    void updateOkButton();

    PtclEditor::TextureImportSettings buildSettings() const;

    void setupFormatControls();
    void setupAdjustmentControls();
    void setupPreviewWidgets();
    void setupConnections();
    QHBoxLayout* buildImportZoomRow();
    QHBoxLayout* buildFormatZoomRow();

private:
    EnumComboBox<Ptcl::TextureFormat> mFormatSelector{};
    EnumComboBox<ImageUtil::ETC1Quality> mETCQuality{};
    QCheckBox mETCDither{};

    QGroupBox mAdjustGroup{};
    EnumComboBox<PtclEditor::TextureAdjustMode> mAdjustMode{};
    QComboBox mTargetWidth{};
    QComboBox mTargetHeight{};
    EnumComboBox<Qt::TransformationMode> mFilter{};

    CropWidget mImportPreview{};
    ThumbnailWidget mFormatPreview{};
    QPushButton mImportZoomIn{};
    QPushButton mImportZoomOut{};
    QPushButton mImportZoomReset{};
    QPushButton mImportResetCrop{};
    QPushButton mFormatZoomIn{};
    QPushButton mFormatZoomOut{};
    QPushButton mFormatZoomReset{};

    QLabel mInfoLabel{};

    QImage mImage{};
    std::unique_ptr<Ptcl::Texture> mTexture{};
    QFutureWatcher<TextureResult> mWatcher{};
    LoadingSpinner mLoadingSpinner{};
    PtclEditor::TextureImportProcessor mProcessor{};
    QPushButton* mOkButton{nullptr};
    std::atomic<bool> mCanceled{false};
};


// ========================================================================== //
